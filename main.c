#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "raylib.h"
#include "raymath.h"

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

    // There are probably way better things to seed from
    SetRandomSeed((int)(&moveSpeed));
    // The chance an ant will turn is 1/turnChanceDenominator
    int turnChanceDenominator = 3;
    int antCount = 60;
    int aliveAnts = antCount;
    int roomRadius = 4;
    ant ants[antCount];
    // The index that an ant collides with is stored here
    int collisions[antCount];
    // Place the ants randomly
    for (int i = 0; i < antCount; i++)
    {
        int a = ((mapRadius / 2) - roomRadius - 5);
        int q = GetRandomValue(-a, a);
        int r = GetRandomValue(-a - (q * (q < 0)), a - (q * (q > 0)));

        ants[i] = (ant){(hexCoord){q, r, -q - r}, GetRandomValue(0, 5), true};
        printf("ant %d: q: %d, r: %d, s: %d\n", i, ants[i].position.q, ants[i].position.r, ants[i].position.s);
    }

    // Set all tiles to -1 to indicate that no ant has been there
    for (int i = 0; i < mapRadius * 2; i++)
    {
        for (int j = 0; j < mapRadius * 2; j++)
        {
            SetTile(IndexToHexCoord(i, j), -1);
        }
    }

    // First pass of terrain generation
    while (aliveAnts > 0)
    {
        for (int i = 0; i < antCount; i++)
        {
            if (ants[i].alive)
            {
                // Determine if the ant should turn
                if (GetRandomValue(0, turnChanceDenominator) == 0)
                {
                    ants[i].direction += (GetRandomValue(0, 1) == 0 ? -1 : 1);
                }
                ants[i].direction %= 6;

                // Move the ant
                ants[i].position = HexCoordAdd(ants[i].position, directionToCoords[ants[i].direction]);

                // If the ant is out of bounds, turn around
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

                // If the rest of the code works this should be redundant but the the issue could be hard to find without these console messages
                // FIX HERE
                if (abs(ants[i].position.q) > mapRadius / 2)
                {
                    ants[i].alive = false;
                    printf("ant escaped q%d\n", ants[i].position.q);
                    continue;
                }
                if (abs(ants[i].position.r) > mapRadius / 2)
                {
                    ants[i].alive = false;
                    printf("ant escaped r%d\n", ants[i].position.r);
                    continue;
                }
                if (abs(ants[i].position.s) > mapRadius / 2)
                {
                    ants[i].alive = false;
                    printf("ant escaped s%d\n", ants[i].position.s);
                    continue;
                }

                switch (GetTile(ants[i].position))
                {
                case -1:
                {
                    // If the ant is on an unexplored tile, set the tile to the ant's index
                    SetTile(ants[i].position, i);
                }
                break;
                default:
                {
                    if (GetTile(ants[i].position) != i)
                    {
                        // If the ant is on a tile that has been explored by another ant, kill the ant, track the collision and set the tile to -2 for a room to be created later
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
        printf("%d ", collisions[i]);
    }
    printf("\n");

    // Second pass of terrain generation
    // Calculate the amount of separated networks of ant trails by replacing the value of each collision with the value at the index the value points to
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

    // Reanimate one ant from each network
    for (int i = 0; i < antCount; i++)
    {
        ants[collisions[i]].alive = true;
    }

    aliveAnts = 0;
    for (int i = 0; i < antCount; i++)
    {
        aliveAnts += ants[i].alive;
    }
    printf("aliveAnts: %d\n", aliveAnts);

    // Set all non relevant collisions to -1 to avoid double updating an ant. This shouldn't be neccesary with the chosen method of connecting networks
    // FIX HERE
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
            printf("%d ", collisions[i]);
        }
    }
    printf("\n");

    // Move all alive ants to the center of the map
    for (int i = 0; i < antCount; i++)
    {
        if (collisions[i] >= 0)
        {
            ant a = ants[collisions[i]];
            while (a.alive)
            {
                printf("updating ant %d ", collisions[i]);
                if (a.position.q != 0 && a.position.r != 0)
                {
                    // Finding the way towards the center of the map
                    hexCoord b = (hexCoord){
                        a.position.q > 0 ? -1 : 1,
                        a.position.r > 0 ? -1 : 1,
                        a.position.s > 0 ? -1 : 1};
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
                    a.position = HexCoordAdd(a.position, b);
                    printf("q: %d, r: %d, s: %d\n", a.position.q, a.position.r, a.position.s);

                    if (GetTile(a.position) == -1)
                    {
                        SetTile(a.position, collisions[i]);
                    }
                }
                else
                {
                    a.alive = false;
                }
            }
        }
    }
    puts("done");

    // Interpret the map
    for (int i = 0; i < mapRadius; i++)
    {
        for (int j = 0; j < mapRadius; j++)
        {
            switch (GetTile(IndexToHexCoord(i, j)))
            {
            case -1:
                // All unexplored tiles are walls
                SetTile(IndexToHexCoord(i, j), TILETYPE_WALL);
                break;
            case -2:
            {
                // All collisions are rooms
                // Room for optimization here by only accessing the tiles that are within the room radius
                hexCoord a = IndexToHexCoord(i, j);
                for (int k = 0; k < mapRadius; k++)
                {
                    for (int l = 0; l < mapRadius; l++)
                    {
                        hexCoord b = IndexToHexCoord(k, l);
                        if (
                            abs(a.q - b.q) < roomRadius &&
                            abs(a.r - b.r) < roomRadius &&
                            abs(a.s - b.s) < roomRadius &&
                            abs(b.q) < mapRadius / 2 &&
                            abs(b.r) < mapRadius / 2 &&
                            abs(b.s) < mapRadius / 2)
                        {
                            SetTile(b, TILETYPE_FLOOR);
                        }
                    }
                }

                // SetTile(IndexToHexCoord(i, j), TILETYPE_HOLE);
            }
            break;
            default:
                SetTile(IndexToHexCoord(i, j), TILETYPE_FLOOR);
                break;
            }
        }
    }
    // Set the player's tile to floor
    SetTile((hexCoord){0, 0, 0}, TILETYPE_FLOOR);

    hexCoord player = (hexCoord){0, 0, 0};
    hexCoord oldPlayer = (hexCoord){0, 0, 0};
    // 1 means the player is at the new position, 0 means the player is at the old position
    float moveLerp = 1;

    while (!WindowShouldClose())
    {
        // -----
        // Input
        // -----
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

        if (moveLerp >= 1)
        {
            if (IsKeyDown(KEY_S) &&
                GetTile(HexCoordAdd(player, directionToCoords[0])) != TILETYPE_WALL)
            {
                oldPlayer = player;
                player = HexCoordAdd(player, directionToCoords[0]);
                moveLerp = 0;
            }
            if (IsKeyDown(KEY_D) &&
                GetTile(HexCoordAdd(player, directionToCoords[1])) != TILETYPE_WALL)
            {
                oldPlayer = player;
                player = HexCoordAdd(player, directionToCoords[1]);
                moveLerp = 0;
            }
            if (IsKeyDown(KEY_E) &&
                GetTile(HexCoordAdd(player, directionToCoords[2])) != TILETYPE_WALL)
            {
                oldPlayer = player;
                player = HexCoordAdd(player, directionToCoords[2]);
                moveLerp = 0;
            }
            if (IsKeyDown(KEY_W) &&
                GetTile(HexCoordAdd(player, directionToCoords[3])) != TILETYPE_WALL)
            {
                oldPlayer = player;
                player = HexCoordAdd(player, directionToCoords[3]);
                moveLerp = 0;
            }
            if (IsKeyDown(KEY_Q) &&
                GetTile(HexCoordAdd(player, directionToCoords[4])) != TILETYPE_WALL)
            {
                oldPlayer = player;
                player = HexCoordAdd(player, directionToCoords[4]);
                moveLerp = 0;
            }
            if (IsKeyDown(KEY_A) &&
                GetTile(HexCoordAdd(player, directionToCoords[5])) != TILETYPE_WALL)
            {
                oldPlayer = player;
                player = HexCoordAdd(player, directionToCoords[5]);
                moveLerp = 0;
            }
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

        // Lerp the camera for smother movement
        if (moveLerp < 1)
        {
            moveLerp += GetFrameTime() * 5;
            cameraPos = Vector2Lerp(
                Vector2Add(Vector2Scale(HexCoordToVector(oldPlayer), -1),
                    (Vector2){GetScreenWidth() / 2, GetScreenHeight() / 2}),
                Vector2Add(Vector2Scale(HexCoordToVector(player), -1),
                    (Vector2){GetScreenWidth() / 2, GetScreenHeight() / 2}),
                moveLerp
            );
        }
        else
        {
            moveLerp = 1;
            cameraPos = Vector2Add(Vector2Scale(HexCoordToVector(player), -1),
                (Vector2){GetScreenWidth() / 2, GetScreenHeight() / 2});
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw the tiles within the player's vision
        // Room for optimization here by only accessing the tiles that are within the player's vision
        int visionRadius = 7;
        // First pass for floor tiles
        for (int k = 0; k < mapRadius; k++)
        {
            for (int l = 0; l < mapRadius; l++)
            {
                hexCoord b = IndexToHexCoord(k, l);
                if (
                    abs(player.q - b.q) < visionRadius &&
                    abs(player.r - b.r) < visionRadius &&
                    abs(player.s - b.s) < visionRadius &&
                    abs(b.q) <= mapRadius / 2 &&
                    abs(b.r) <= mapRadius / 2 &&
                    abs(b.s) <= mapRadius / 2)
                {
                    DrawPoly(HexCoordToCameraVector(IndexToHexCoord(k, l)),
                        6, tileRadius, 30, tileColors[GetTile(IndexToHexCoord(k, l))]);
                    DrawPolyLines(HexCoordToCameraVector(IndexToHexCoord(k, l)),
                        6, tileRadius, 30, BLACK);
                }
            }
        }
        // Player
        DrawCircleV(
            Vector2Lerp(HexCoordToCameraVector(oldPlayer),
                HexCoordToCameraVector(player), moveLerp),
            tileRadius * 0.8, (Color){255, 0, 0, 255});

        // Second pass for the walls' walls
        for (int i = 0; i < mapRadius; i++)
        {
            for (int j = 0; j < mapRadius; j++)
            {
                hexCoord b = IndexToHexCoord(i, j);
                if (
                    abs(player.q - b.q) < visionRadius &&
                    abs(player.r - b.r) < visionRadius &&
                    abs(player.s - b.s) < visionRadius &&
                    abs(b.q) <= mapRadius / 2 &&
                    abs(b.r) <= mapRadius / 2 &&
                    abs(b.s) <= mapRadius / 2)
                {
                    if (GetTile(IndexToHexCoord(i, j)) == TILETYPE_WALL)
                    {
                        DrawPoly(
                            HexCoordToCameraVector(IndexToHexCoord(i, j)),
                            6, tileRadius, 30, tileColors[TILETYPE_WALL]);
                        DrawPolyLines(
                            HexCoordToCameraVector(IndexToHexCoord(i, j)),
                            6, tileRadius, 30, BLACK);
                        DrawRectangleV(
                            Vector2Add((Vector2){-tileRadius, -tileRadius * 0.5},
                                HexCoordToCameraVector(IndexToHexCoord(i, j))),
                            (Vector2){tileRadius * 2, tileRadius * 0.5},
                            tileColors[TILETYPE_WALL]);
                    }
                }
            }
        }
        // Third pass for the top of the walls
        for (int i = 0; i < mapRadius; i++)
        {
            for (int j = 0; j < mapRadius; j++)
            {
                hexCoord b = IndexToHexCoord(i, j);
                if (abs(b.q) <= mapRadius / 2 && 
                    abs(b.r) <= mapRadius / 2 && 
                    abs(b.s) <= mapRadius / 2)
                {
                    if (GetTile(IndexToHexCoord(i, j)) == TILETYPE_WALL)
                    {
                        if (
                            abs(player.q - b.q) < visionRadius &&
                            abs(player.r - b.r) < visionRadius && 
                            abs(player.s - b.s) < visionRadius)
                        {
                            DrawPoly(
                                Vector2Add((Vector2){0, -tileRadius * 0.5},
                                    HexCoordToCameraVector(IndexToHexCoord(i, j))),
                                6, tileRadius, 30, (Color){200, 150, 0, 255});
                            DrawPolyLines(
                                Vector2Add((Vector2){0, -tileRadius * 0.5},
                                    HexCoordToCameraVector(IndexToHexCoord(i, j))),
                                6, tileRadius, 30, BLACK);
                        }
                    }
                }
            }
        }
        /* for (int i = 0; i < mapRadius; i++)
        {
            for (int j = 0; j < mapRadius; j++)
            {
                // if (abs(IndexToHexCoord(i,j).s) < mapRadius/2+1)
                // {
                //     DrawPoly(HexCoordToCameraVector(IndexToHexCoord(i, j)), 6, tileRadius, 30, tileColors[GetTile(IndexToHexCoord(i, j))]);
                //     DrawPolyLines(HexCoordToCameraVector(IndexToHexCoord(i, j)), 6, tileRadius, 30, BLACK);
                // }

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

        DrawFPS(10, 30);

        EndDrawing();
    }
    return 0;
}