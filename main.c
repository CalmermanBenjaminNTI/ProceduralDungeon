#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "raylib.h"
#include "raymath.h"

// Sebbe's suggestion
/* typedef enum tile_state_e {
    TILE_STATE_NONE,
    TILE_STATE_VISITED,
    TILE_STATE_VISIBLE
} tile_state_e;

typedef enum tile_type_e {
    TILE_TYPE_NONE,
    TILE_TYPE_FLOOR,
    TILE_TYPE_WALL,
} tile_type_e:


typedef struct tile_t {
    tile_state_e state;
    tile_type_e type;
} tile_t;
 */

typedef enum TILETYPE
{
    TILETYPE_NONE,
    TILETYPE_FLOOR,
    TILETYPE_WALL,
    TILETYPE_HOLE
} TILETYPE;

typedef struct hexCoord
{
    int q;
    int r;
    int s;
} hexCoord;

const int startMapRadius = 101; // Must be odd
TILETYPE map[startMapRadius * 2][startMapRadius * 2];
int mapRadius = startMapRadius;
float tileRadius = 80;
Vector2 cameraPos = (Vector2){0, 0};

TILETYPE GetTile(hexCoord coord)
{
    return map[abs(coord.q * 2) - (coord.q > 0)][abs(coord.r * 2) - (coord.r > 0)];
}

void SetTile(hexCoord coord, TILETYPE tile)
{
    map[abs(coord.q * 2) - (coord.q > 0)][abs(coord.r * 2) - (coord.r > 0)] = tile;
}

Vector2 HexCoordToVector(hexCoord coord)
{
    float x = tileRadius * 1.5 * coord.q;
    float y = (coord.s - coord.r) * sqrt(3) * tileRadius * 0.5;
    return (Vector2){x, y};
}

Vector2 HexCoordToCameraVector(hexCoord coord)
{
    float x = tileRadius * 1.5 * coord.q + cameraPos.x;
    float y = (coord.s - coord.r) * sqrt(3) * tileRadius * 0.5 + cameraPos.y;
    return (Vector2){x, y};
}

hexCoord IndexToHexCoord(int q, int r)
{
    q = (q % 2 == 0 ? -q : q + 1) / 2;
    r = (r % 2 == 0 ? -r : r + 1) / 2;
    return (hexCoord){q, r, -q - r};
}

hexCoord HexCoordAdd(hexCoord a, hexCoord b)
{
    return (hexCoord){a.q + b.q, a.r + b.r, a.s + b.s};
}

hexCoord hexCoordSubtract(hexCoord a, hexCoord b)
{
    return (hexCoord){a.q - b.q, a.r - b.r, a.s - b.s};
}

typedef struct ant
{
    hexCoord position;
    int direction;
    bool alive;
} ant;

int main()
{
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    InitWindow(screenWidth, screenHeight, "endless dungeon");
    ToggleFullscreen();

    float moveSpeed = 500;

    Color tileColors[4] = {
        (Color){0, 0, 0, 0},
        (Color){220, 200, 50, 255},
        (Color){100, 80, 0, 255},
        (Color){100, 100, 255, 255}};

    const hexCoord directionToCoords[6] = {
        (hexCoord){0, -1, 1},
        (hexCoord){1, -1, 0},
        (hexCoord){1, 0, -1},
        (hexCoord){0, 1, -1},
        (hexCoord){-1, 1, 0},
        (hexCoord){-1, 0, 1}};

    SetRandomSeed(4);
    int turnChanceDenominator = 3;
    int antCount = 60;
    int aliveAnts = antCount;
    int roomRadius = 4;
    ant ants[antCount];
    int collisions[antCount];
    for (int i = 0; i < antCount; i++)
    {
        int a = ((mapRadius / 2) - roomRadius - 5);
        int q = GetRandomValue(-a, a);
        int r = GetRandomValue(-a - (q * (q < 0)), a - (q * (q > 0)));

        ants[i] = (ant){(hexCoord){q, r, -q - r}, GetRandomValue(0, 5), true};
        // printf("ant %d: q: %d, r: %d, s: %d\n", i, ants[i].position.q, ants[i].position.r, ants[i].position.s);
    }

    for (int i = 0; i < mapRadius * 2; i++)
    {
        for (int j = 0; j < mapRadius * 2; j++)
        {
            SetTile(IndexToHexCoord(i, j), -1);
        }
    }

    while (aliveAnts > 0)
    {
        for (int i = 0; i < antCount; i++)
        {
            if (ants[i].alive)
            {

                if (GetRandomValue(0, turnChanceDenominator) == 0)
                {
                    ants[i].direction += (GetRandomValue(0, 1) == 0 ? -1 : 1);
                }
                ants[i].direction %= 6;

                ants[i].position = HexCoordAdd(ants[i].position, directionToCoords[ants[i].direction]);

                if (
                    abs(ants[i].position.q) > mapRadius / 2 - 1 ||
                    abs(ants[i].position.r) > mapRadius / 2 - 1 ||
                    abs(ants[i].position.s) > mapRadius / 2 - 1)
                {
                    ants[i].position.q -= directionToCoords[ants[i].direction].q;
                    ants[i].position.r -= directionToCoords[ants[i].direction].r;
                    ants[i].position.s -= directionToCoords[ants[i].direction].s;
                    ants[i].direction = (ants[i].direction + 3) % 6;
                }

                if (abs(ants[i].position.q) > mapRadius / 2)
                {
                    ants[i].alive = false;
                    // printf("ant escaped q%d\n", ants[i].position.q);
                    continue;
                }
                if (abs(ants[i].position.r) > mapRadius / 2)
                {
                    ants[i].alive = false;
                    // printf("ant escaped r%d\n", ants[i].position.r);
                    continue;
                }
                if (abs(ants[i].position.s) > mapRadius / 2)
                {
                    ants[i].alive = false;
                    // printf("ant escaped s%d\n", ants[i].position.s);
                    continue;
                }

                switch (GetTile(ants[i].position))
                {
                case -1:
                {
                    SetTile(ants[i].position, i);
                }
                break;
                default:
                {
                    if (GetTile(ants[i].position) != i)
                    {
                        ants[i].alive = false;
                        puts("ant died");
                        collisions[i] = GetTile(ants[i].position);
                        SetTile(ants[i].position, -2);
                    }
                }
                break;
                }
            }
        }

        aliveAnts = 0;
        for (int i = 0; i < antCount; i++)
        {
            aliveAnts += ants[i].alive;
        }
    }

    for (int i = 0; i < antCount; i++)
    {
        // printf("%d ", collisions[i]);
    }
    // printf("\n");

    for (int i = 0; i < antCount; i++)
    {
        for (int j = 0; j < antCount; j++)
        {
            if (collisions[j] == i)
            {
                collisions[j] = collisions[i];
            }
        }
    }
    for (int i = 0; i < antCount; i++)
    {
        SetTile(ants[collisions[i]].position, -2);
        ants[collisions[i]].alive = true;
    }

    aliveAnts = 0;
    for (int i = 0; i < antCount; i++)
    {
        aliveAnts += ants[i].alive;
    }
    // printf("aliveAnts: %d\n", aliveAnts);

    for (int i = 0; i < antCount; i++)
    {
        for (int j = i + 1; j < antCount; j++)
        {
            if (collisions[j] == collisions[i] && collisions[j] >= 0)
            {
                collisions[j] = -collisions[j];
            }
        }
    }

    for (int i = 0; i < antCount; i++)
    {
        if (collisions[i] >= 0)
        {
            // printf("%d ", collisions[i]);
        }
    }
    // printf("\n");

    for (int i = 0; i < antCount; i++)
    {
        if (collisions[i] >= 0)
        {
            ant a = ants[collisions[i]];
            while (a.alive)
            {
                // printf("updating ant %d ", collisions[i]);
                if (a.position.q != 0 && a.position.r != 0)
                {
                    hexCoord b = (hexCoord){a.position.q > 0 ? -1 : 1, a.position.r > 0 ? -1 : 1, a.position.s > 0 ? -1 : 1};
                    if (abs(a.position.q) < abs(a.position.r))
                    {
                        if (abs(a.position.q) < abs(a.position.s))
                        {
                            b.q = 0;
                        }
                        else
                        {
                            b.s = 0;
                        }
                    }
                    else
                    {
                        if (abs(a.position.r) < abs(a.position.s))
                        {
                            b.r = 0;
                        }
                        else
                        {
                            b.s = 0;
                        }
                    }
                    a.position.q += b.q;
                    a.position.r += b.r;
                    a.position.s += b.s;
                    // printf("q: %d, r: %d, s: %d\n", a.position.q, a.position.r, a.position.s);

                    switch (GetTile(a.position))
                    {
                    case -1:
                    {
                        SetTile(a.position, collisions[i]);
                    }
                    break;
                    case -2:
                        break;
                    default:
                    {
                        /* if (abs(collisions[GetTile(a.position)]) != i)
                        {
                            //a.alive = false;
                            //puts("ant died");
                            SetTile(a.position, -3);
                        } */
                    }
                    }

                    if (abs(a.position.q) > mapRadius / 2)
                    {
                        a.alive = false;
                        // printf("ant escaped q%d\n", a.position.q);
                        continue;
                    }
                    if (abs(a.position.r) > mapRadius / 2)
                    {
                        a.alive = false;
                        // printf("ant escaped r%d\n", a.position.r);
                        continue;
                    }
                    if (abs(a.position.s) > mapRadius / 2)
                    {
                        a.alive = false;
                        // printf("ant escaped s%d\n", a.position.s);
                        continue;
                    }
                }
                else
                {
                    a.alive = false;
                }
            }
        }
    }
    // puts("done");

    for (int i = 0; i < mapRadius; i++)
    {
        for (int j = 0; j < mapRadius; j++)
        {
            switch (GetTile(IndexToHexCoord(i, j)))
            {
            case -1:
                SetTile(IndexToHexCoord(i, j), TILETYPE_WALL);
                break;
            case -2:
            {
                hexCoord a = IndexToHexCoord(i, j);
                for (int k = 0; k < mapRadius; k++)
                {
                    for (int l = 0; l < mapRadius; l++)
                    {
                        hexCoord b = IndexToHexCoord(k, l);
                        if (abs(a.q - b.q) < roomRadius && abs(a.r - b.r) < roomRadius && abs(a.s - b.s) < roomRadius && abs(b.q) < mapRadius / 2 && abs(b.r) < mapRadius / 2 && abs(b.s) < mapRadius / 2)
                        {
                            SetTile(b, TILETYPE_FLOOR);
                        }
                    }
                }

                // SetTile(IndexToHexCoord(i, j), TILETYPE_HOLE);
            }
            break;
            case -3:
                SetTile(IndexToHexCoord(i, j), TILETYPE_NONE);
                break;
            default:
                SetTile(IndexToHexCoord(i, j), TILETYPE_FLOOR);
                break;
            }
        }
    }

    hexCoord player = (hexCoord){0, 0, 0};
    hexCoord oldPlayer = (hexCoord){0, 0, 0};
    float moveLerp = 1;

    const int viewRadius = 6;
    const int viewArcsCount = 3 * viewRadius * (viewRadius + 1) / 2;
    printf("View arc count %d\n", viewArcsCount);
    Vector2 viewArcs[viewArcsCount];
    for (int i = 0; i < viewArcsCount; i++)
    {
        viewArcs[i] = (Vector2){-1, -1};
    }
    bool reRenderFOV = true;

    hexCoord tilesToDraw[6 * viewRadius * (viewRadius + 1) / 2 + 1];

    while (!WindowShouldClose())
    {
        if (IsKeyDown(KEY_UP))
        {
            cameraPos.y += moveSpeed * GetFrameTime();
        }
        if (IsKeyDown(KEY_DOWN))
        {
            cameraPos.y -= moveSpeed * GetFrameTime();
        }
        if (IsKeyDown(KEY_LEFT))
        {
            cameraPos.x += moveSpeed * GetFrameTime();
        }
        if (IsKeyDown(KEY_RIGHT))
        {
            cameraPos.x -= moveSpeed * GetFrameTime();
        }
        if (IsKeyDown(KEY_U))
        {
            tileRadius *= 1 + GetFrameTime();
        }
        if (IsKeyDown(KEY_J))
        {
            tileRadius *= 1 - GetFrameTime();
        }

        if (IsKeyDown(KEY_S) && GetTile(HexCoordAdd(player, directionToCoords[0])) != TILETYPE_WALL && moveLerp >= 1)
        {
            oldPlayer = player;
            player = HexCoordAdd(player, directionToCoords[0]);
            moveLerp = 0;
        }
        if (IsKeyDown(KEY_D) && GetTile(HexCoordAdd(player, directionToCoords[1])) != TILETYPE_WALL && moveLerp >= 1)
        {
            oldPlayer = player;
            player = HexCoordAdd(player, directionToCoords[1]);
            moveLerp = 0;
        }
        if (IsKeyDown(KEY_E) && GetTile(HexCoordAdd(player, directionToCoords[2])) != TILETYPE_WALL && moveLerp >= 1)
        {
            oldPlayer = player;
            player = HexCoordAdd(player, directionToCoords[2]);
            moveLerp = 0;
        }
        if (IsKeyDown(KEY_W) && GetTile(HexCoordAdd(player, directionToCoords[3])) != TILETYPE_WALL && moveLerp >= 1)
        {
            oldPlayer = player;
            player = HexCoordAdd(player, directionToCoords[3]);
            moveLerp = 0;
        }
        if (IsKeyDown(KEY_Q) && GetTile(HexCoordAdd(player, directionToCoords[4])) != TILETYPE_WALL && moveLerp >= 1)
        {
            oldPlayer = player;
            player = HexCoordAdd(player, directionToCoords[4]);
            moveLerp = 0;
        }
        if (IsKeyDown(KEY_A) && GetTile(HexCoordAdd(player, directionToCoords[5])) != TILETYPE_WALL && moveLerp >= 1)
        {
            oldPlayer = player;
            player = HexCoordAdd(player, directionToCoords[5]);
            moveLerp = 0;
        }

        if (moveLerp == 0)
        {
            reRenderFOV = true;
        }

        /* for (int i = 0; i < 20; i++)
        {
            for (int j = 0; j < 20; j++)
            {
                printf("i: %d, j: %d ", i, j);
                hexCoord a = IndexToHexCoord(i, j);
                printf("q: %d, r: %d, s: %d\n", a.q, a.r, a.s);
            }

        } */

        if (moveLerp < 1)
        {
            moveLerp += GetFrameTime() * 5;
            cameraPos = Vector2Lerp(Vector2Add(Vector2Scale(HexCoordToVector(oldPlayer), -1), (Vector2){GetScreenWidth() / 2, GetScreenHeight() / 2}), Vector2Add(Vector2Scale(HexCoordToVector(player), -1), (Vector2){GetScreenWidth() / 2, GetScreenHeight() / 2}), moveLerp);
        }
        else
        {
            moveLerp = 1;
            cameraPos = Vector2Add(Vector2Scale(HexCoordToVector(player), -1), (Vector2){GetScreenWidth() / 2, GetScreenHeight() / 2});
        }

        BeginDrawing();
        ClearBackground(BLACK);

        int tileCount;

        // Visit every tile within i radius
        if (reRenderFOV)
        {
            // Reset viewArcs
            for (int i = 0; i < viewArcsCount; i++)
            {
                viewArcs[i] = (Vector2){-1, -1};
            }
            puts("");
            printf("Player: %d, %d, %d\n", player.q, player.r, player.s);
            int arcIndex = 0;
            // For every ring until the view radius
            for (int i = 1; i < viewRadius; i++)
            {
                printf("Ring: %d\n", i);
                // Initial tile to check
                hexCoord a = (hexCoord){-i, i, 0};
                // For each side of the hexagonal area
                for (int j = 0; j < 6; j++)
                {
                    // Check as many tiles as the radius of the ring
                    for (int k = 0; k < i; k++)
                    {
                        // Walk around the ring
                        // Next tile to check
                        hexCoord b = HexCoordAdd(a, directionToCoords[j]);
                        
                        // The actual tile position
                        hexCoord c = HexCoordAdd(a,player);
                        printf("a: %d, %d, %d c: %d, %d, %d\n", a.q, a.r, a.s, c.q, c.r, c.s);

                        // If the tile is a wall
                        if (GetTile(HexCoordAdd(a, player)) == TILETYPE_WALL)
                        {
                            // If the second value of the arc is unassigned assign it to the angle of the tile
                            if (viewArcs[arcIndex].y == -1)
                            {
                                viewArcs[arcIndex].y = atan2(HexCoordToVector(a).x, HexCoordToVector(a).y); // + 2*PI*(atan2(HexCoordToVector(a).x, HexCoordToVector(a).y) < 0);
                            }

                            // If the following tile is not a wall set the first value of the next arc to the angle of the tile
                            if (GetTile(HexCoordAdd(b, player)) != TILETYPE_WALL)
                            {
                                arcIndex++;
                                viewArcs[arcIndex].x = atan2(HexCoordToVector(a).x, HexCoordToVector(a).y); // + 2*PI*(atan2(HexCoordToVector(a).x, HexCoordToVector(a).y) < 0);
                            }
                        }
                        // If the tile is the last tile of the last edge of this ring
                        if (k == i - 1 && j == 5)
                        {
                            // If the the first value of the arc is unassigned there has only been one arc and so it is a fully clear ring
                            if (viewArcs[arcIndex].x == -1)
                            {
                                // Set both values to -2 to indicate that the ring is clear
                                viewArcs[3 * (i - 1) * i / 2] = (Vector2){-2, -2};
                                printf("clear, arcIndex: %d 0:%d\n", arcIndex, 3 * (i - 1) * i / 2);
                            }
                            else
                            {
                                // If the first value of the arc is assigned there are at least two arcs and so the first arc's first value should be set to the last arc's second value
                                viewArcs[3 * (i - 1) * i / 2].x = viewArcs[arcIndex].x;
                                viewArcs[arcIndex].x = -1;
                            }
                        }
                        // Check the next tile
                        a = b;
                    }
                }
                // Move to the first arc of the next ring
                arcIndex = 3 * i * (i + 1) / 2;
            }
            // For all view arcs
            for (int j = 0; j < viewArcsCount; j++)
            {
                switch (j)
                {
                case 3:
                case 9:
                case 18:
                case 30:
                case 45:
                case 63:
                case 84:
                case 108:
                case 135:
                case 165:
                    printf("Next circle, arcIndex: %d\n", j);
                    break;
                default:
                    break;
                }
                printf("%d: %f, %f\n", j, RAD2DEG * viewArcs[j].x, RAD2DEG * viewArcs[j].y);
            }
            tilesToDraw[0] = player;
            for (int i = 0; i < 6; i++)
            {
                tilesToDraw[i + 1] = HexCoordAdd(player, directionToCoords[i]);
            }
            tileCount = 7;
            for (int i = 2; i < viewRadius; i++)
            {
                hexCoord a = (hexCoord){-i, i, 0};
                for (int j = 0; j < 6; j++)
                {
                    for (int k = 0; k < i; k++)
                    {
                        float angle = atan2(HexCoordToVector(a).x, HexCoordToVector(a).y);
                        for (int l = 3 * (i - 1) * i / 2; l < 3 * i * (i + 1) / 2; l++)
                        {
                            if (viewArcs[l].x != -1 && viewArcs[l].y != -1)
                            {
                                if ((angle >= viewArcs[l].x && angle <= viewArcs[l].y) || (viewArcs[l].x == -2 && viewArcs[l].y == -2))
                                {
                                    tilesToDraw[tileCount] = HexCoordAdd(a, player);
                                    tileCount++;
                                    break;
                                }
                            }
                        }
                        a = HexCoordAdd(a, directionToCoords[j]);
                    }
                }
            }
            reRenderFOV = false;
        }

        for (int i = 0; i < tileCount; i++)
        {
            DrawPoly(HexCoordToCameraVector(tilesToDraw[i]), 6, tileRadius, 30, tileColors[GetTile(tilesToDraw[i])]);
            DrawPolyLines(HexCoordToCameraVector(tilesToDraw[i]), 6, tileRadius, 30, BLACK);
        }
        DrawCircleV(Vector2Lerp(HexCoordToCameraVector(oldPlayer), HexCoordToCameraVector(player), moveLerp), tileRadius * 0.8, (Color){255, 0, 0, 255});
        for (int i = 0; i < tileCount; i++)
        {
            if (GetTile(tilesToDraw[i]) == TILETYPE_WALL)
            {
                // DrawPoly(HexCoordToCameraVector(tilesToDraw[i]), 6, tileRadius, 30, tileColors[TILETYPE_WALL]);
                // DrawPolyLines(HexCoordToCameraVector(tilesToDraw[i]), 6, tileRadius, 30, BLACK);
                DrawRectangleV(Vector2Add((Vector2){-tileRadius, -tileRadius * 0.5}, HexCoordToCameraVector(tilesToDraw[i])), (Vector2){tileRadius * 2, tileRadius * 0.5}, tileColors[TILETYPE_WALL]);
            }
        }
        for (int i = 0; i < tileCount; i++)
        {
            if (GetTile(tilesToDraw[i]) == TILETYPE_WALL)
            {
                DrawPoly(Vector2Add((Vector2){0, -tileRadius * 0.5}, HexCoordToCameraVector(tilesToDraw[i])), 6, tileRadius, 30, (Color){200, 150, 0, 255});
                DrawPolyLines(Vector2Add((Vector2){0, -tileRadius * 0.5}, HexCoordToCameraVector(tilesToDraw[i])), 6, tileRadius, 30, BLACK);
            }
        }

        /* int visionRadius = 21;
        for (int k = 0; k < mapRadius; k++)
        {
            for (int l = 0; l < mapRadius; l++)
            {
                hexCoord b = IndexToHexCoord(k, l);
                if (abs(player.q - b.q) < visionRadius && abs(player.r - b.r) < visionRadius && abs(player.s - b.s) < visionRadius && abs(b.q) <= mapRadius/2 && abs(b.r) <= mapRadius/2 && abs(b.s) <= mapRadius/2)
                {
                    DrawPoly(HexCoordToCameraVector(IndexToHexCoord(k, l)), 6, tileRadius, 30, tileColors[GetTile(IndexToHexCoord(k, l))]);
                    DrawPolyLines(HexCoordToCameraVector(IndexToHexCoord(k, l)), 6, tileRadius, 30, BLACK);
                }
            }
        } */
        // DrawCircleV(Vector2Lerp(HexCoordToCameraVector(oldPlayer),HexCoordToCameraVector(player),moveLerp), tileRadius*0.8, (Color){255,0,0,255});
        /* for (int i = 0; i < mapRadius; i++)
        {
            for (int j = 0; j < mapRadius; j++)
            {
                hexCoord b = IndexToHexCoord(i, j);
                if (abs(player.q - b.q) < visionRadius && abs(player.r - b.r) < visionRadius && abs(player.s - b.s) < visionRadius && abs(b.q) <= mapRadius/2 && abs(b.r) <= mapRadius/2 && abs(b.s) <= mapRadius/2)
                {
                    if (GetTile(IndexToHexCoord(i, j)) == TILETYPE_WALL)
                    {
                        DrawPoly(HexCoordToCameraVector(IndexToHexCoord(i,j)), 6, tileRadius, 30, tileColors[TILETYPE_WALL]);
                        DrawPolyLines(HexCoordToCameraVector(IndexToHexCoord(i, j)), 6, tileRadius, 30, BLACK);
                        DrawRectangleV(Vector2Add((Vector2){-tileRadius,-tileRadius*0.5},HexCoordToCameraVector(IndexToHexCoord(i, j))), (Vector2){tileRadius*2,tileRadius*0.5}, tileColors[TILETYPE_WALL]);
                    }
                }
            }
        } */
        /* for (int i = 0; i < mapRadius; i++)
        {
            for (int j = 0; j < mapRadius; j++)
            {
                hexCoord b = IndexToHexCoord(i, j);
                if (abs(player.q - b.q) < visionRadius && abs(player.r - b.r) < visionRadius && abs(player.s - b.s) < visionRadius && abs(b.q) <= mapRadius/2 && abs(b.r) <= mapRadius/2 && abs(b.s) <= mapRadius/2)
                {
                    if (GetTile(IndexToHexCoord(i, j)) == TILETYPE_WALL)
                    {
                        DrawPoly(Vector2Add((Vector2){0,-tileRadius*0.5},HexCoordToCameraVector(IndexToHexCoord(i, j))), 6, tileRadius, 30, (Color){200,150,0,255});
                        DrawPolyLines(Vector2Add((Vector2){0,-tileRadius*0.5},HexCoordToCameraVector(IndexToHexCoord(i, j))), 6, tileRadius, 30, BLACK);
                    }
                }
            }
        } */
        /* for (int i = 0; i < mapRadius; i++)
        {
            for (int j = 0; j < mapRadius; j++)
            {
                if (abs(IndexToHexCoord(i,j).s) < mapRadius/2+1)
                {
                    DrawPoly(HexCoordToCameraVector(IndexToHexCoord(i, j)), 6, tileRadius, 30, tileColors[GetTile(IndexToHexCoord(i, j))]);
                    DrawPolyLines(HexCoordToCameraVector(IndexToHexCoord(i, j)), 6, tileRadius, 30, BLACK);
                }

                // write coordinates of the tile under the mouse
                if (CheckCollisionPointCircle(GetMousePosition(), HexCoordToCameraVector(IndexToHexCoord(i, j)), tileRadius))
                {
                    DrawText(TextFormat("q: %d, r: %d, s: %d", IndexToHexCoord(i, j).q, IndexToHexCoord(i, j).r, IndexToHexCoord(i, j).s), 10, 10, 20, WHITE);
                }
            }
        } */
        // DrawCircleV(HexCoordToCameraVector(player), tileRadius*0.8, (Color){255,0,0,255});
        /* for (int i = 0; i < mapRadius; i++)
        {
            for (int j = 0; j < mapRadius; j++)
            {
                if (abs(IndexToHexCoord(i,j).s) < mapRadius/2+1 && GetTile(IndexToHexCoord(i, j)) == TILETYPE_WALL)
                {
                    DrawRectangleV(Vector2Add((Vector2){-tileRadius,-tileRadius*0.5},HexCoordToCameraVector(IndexToHexCoord(i, j))), (Vector2){tileRadius*2,tileRadius*0.5}, tileColors[GetTile(IndexToHexCoord(i, j))]);
                }
            }
        }
        for (int i = 0; i < mapRadius; i++)
        {
            for (int j = 0; j < mapRadius; j++)
            {
                if (abs(IndexToHexCoord(i,j).s) < mapRadius/2+1 && GetTile(IndexToHexCoord(i, j)) == TILETYPE_WALL)
                {
                    DrawPoly(Vector2Add((Vector2){0,-tileRadius*0.5},HexCoordToCameraVector(IndexToHexCoord(i, j))), 6, tileRadius, 30, (Color){200,150,0,255});
                    DrawPolyLines(Vector2Add((Vector2){0,-tileRadius*0.5},HexCoordToCameraVector(IndexToHexCoord(i, j))), 6, tileRadius, 30, BLACK);
                }
            }
        } */

        // visit every tile within i radius
        for (int i = 1; i < viewRadius; i++)
        {
            hexCoord a = HexCoordAdd(player, (hexCoord){-i, i, 0});
            for (int j = 0; j < 6; j++)
            {
                for (int k = 0; k < i; k++)
                {
                    hexCoord b = HexCoordAdd(a, directionToCoords[j]);
                    DrawText(TextFormat("%d", j * i + k), HexCoordToCameraVector(a).x - tileRadius * 0.5, HexCoordToCameraVector(a).y - tileRadius * 0.8, tileRadius, (Color){0, 0, 0, 20});
                    a = b;
                }
            }
        }

        for (int i = 0; i < viewArcsCount; i++)
        {
            if (viewArcs[i].x != -1)
            {
                if (viewArcs[i].y == -2)
                {
                    DrawRing(HexCoordToCameraVector(player), tileRadius + i * 10, tileRadius + i * 10 + 40, 0, 360, 30, ColorFromHSV(i * 20, 1, 1));
                    DrawRingLines(HexCoordToCameraVector(player), tileRadius + i * 10, tileRadius + i * 10 + 40, 0, 360, 30, BLACK);
                }
                DrawRing(HexCoordToCameraVector(player), tileRadius + i * 10, tileRadius + i * 10 + 40, viewArcs[i].x * RAD2DEG, viewArcs[i].y * RAD2DEG, 30, ColorFromHSV(i * 20, 1, 1));
                DrawRingLines(HexCoordToCameraVector(player), tileRadius + i * 10, tileRadius + i * 10 + 40, viewArcs[i].x * RAD2DEG, viewArcs[i].y * RAD2DEG, 30, BLACK);
            }
        }

        DrawFPS(10, 30);

        EndDrawing();
    }
    return 0;
}