# ProceduralDungeon
School assignment to create a procedurally generated world. This solution is inspired by the drunkard's walk method. To make things interesting this was done on a hex-grid.

The hex grid is stored in a 2d array with index 0 located at the centre of the map. It uses cube coordinates as described by redblobgames in their blog about hex grids and stores them as axial coordinates with every even slot storing a negative coordinate and every odd slot storing a positive coordinate. This allows for the array to be made dynamic and theoretically allows the player to walk infinitely in any direction.
I made the following functions to help with managing the hex grid
- GetTile() Retrieves a tile type from the array with hex coordinates as an argument.
- SetTile() Sets a tile type in the array with hex coordinates and tile type as arguments.
- HexCoordToVector() Converts hex coordinates to a 2d vector that can be used to draw objects on the screen.
- IndexToHexCoord() Calculates the hex coordinates based on the indexes in the 2d array so all tiles can be looped through with 2 for loops.
Later additions were
- HexCoordToCameraVector() This includes the camera's position in the calculations so that the camera can be moved.
- HexCoordAdd() Adds two hexcoords together
- HexCoordSubtract() Subtracts the second argument from the first

# Terrain gen
The terrain gen is inspired by drunkard’s walk. The drunkards will from here on be referred to as ants. An ant has a hex coordinate position, a direction from 0-5 and a boolean for if they are alive. The ants favour to walk straight over turning and when they turn they only turn by +1 or -1 to their direction. When an ant walks where another ant has previously walked it dies and a room is created. The ants have a few global variables that can be adjusted.
- turnChanceDenominator: the probability that the ant decides to turn is 1/this value.
- antCount: the amount of ants.
- roomRadius: the radius of a room.
- Seed: The seed for the ants’ random movement.
## How it works:
1. All map tiles are set to -1 (because -1 will never be an index in the array of ants)
2. All ants are placed randomly on the map
3. All alive ants wander around the map until all have died
  a. The ant generates a random integer between 0 and turnChanceDenominator. If it is 0 it randomly decides to change its direction by -1 or +1.
  b. The direction is run through modulus 6
  c. The ant changes its position using the direction to get a hex coordinate from an array of movements (hexCoordAdd() was added to simplify this) the ant won’t move if that would cause it to end up outside of the the map (with a 1 tile padding to make sure the out-most tiles of the map are walls)
  d. The ant checks what tile type it is standing on
    i. If it is -1 the ant replaces it with its own index in the ant array.
    ii. If it is not -1 and not its own index it dies, goes to the ant’s index in an array of collisions and writes the index of the ant whom’s trail it collided with (as written on the tile) and the ant sets the tile to -2 (to create a room later)
4. After all ants have died the array of collisions is modified to see how many different separate networks of trails have been made. This is done by replacing all 0s with the value of index 0 and then replacing all 1s with the value of index 1 and so on. Eventually there will be one value per separated network.
5. The ants with those indexes become alive and walk towards 0, 0, 0 and set the tiles they walk on to their index.
6. After reaching 0, 0, 0 the ants die again.
7. All tiles with -1 become walls
8. All tiles with -2 set all tiles within the room radius to floor
9. All tiles that are neither -1 or -2 have been walked on by ants and are set to floor

# Rendering / The player
To make everything a bit more fun a player was added, it is a red circle drawn in the middle of the screen that uses simple tile based movement using q,w,e,a,s,d (because of the hex grid). To make the movement smoother the player slides between tiles along witht the camera using the lerp function. The rendering loops through all tiles but only draws them if they are within the view radius of the player. This is optimised to rendering from the player outwards for a set radius in the branch where I tried to add Field of View. The rendering is done in three passes with different layers of textures to give walls depth while also not having to make sure all tiles are drawn from top to bottom of the screen.
