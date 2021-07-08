# Ccraft

It's a minecraft-inspired game written in C using modern OpenGL.

Gameplay video - [Youtube](https://www.youtube.com/watch?v=BgpsSmIqFEc)

These features are implemented:

* Infinite multithreaded world generation
* Biomes, each with unique heightmap
* Player physics (on land and in water)
* Using database to store map and player information
* Building & destroying blocks
* Depth of field effect (smooth / non-smooth)
* Motion blur effect
* Day/night cycles
* Using configuration file to change game settings without recompilation

### Screenshots

![1](https://github.com/Winter091/Ccraft/blob/main/screenshots/biomes.png)
![2](https://github.com/Winter091/Ccraft/blob/main/screenshots/morning.png)
![3](https://github.com/Winter091/Ccraft/blob/main/screenshots/forest.png)
![4](https://github.com/Winter091/Ccraft/blob/main/screenshots/mountains.png)
![5](https://github.com/Winter091/Ccraft/blob/main/screenshots/night-2.png)

### Controls

* WASD                - Move player
* Tab                 - Toggle fly mode
* Shift               - Sneak mode / fly up
* Ctrl                - Run / fly down
* Page up/down (hold) - Increase/decrease fly camera speed
* C                   - Zoom camera
* Left mouse button   - Destroy block
* Right mouse button  - Place block
* Mouse scroll wheel  - Change build block

### How to compile

Build is done using Cmake, so install it.

#### Windows (using mingw32-make)

Download and install [MinGW](https://sourceforge.net/projects/mingw-w64/).

Then in the project root folder run following commands:

    mkdir build && cd build
    cmake .. -G "MinGW Makefiles"
    mingw32-make
    Ccraft

#### Windows (using Visual Studio 2019)

In the project root folder run following commands:

    mkdir build && cd build
    cmake .. -G "Visual Studio 16 2019" -A x64
 
Ccraft.sln will be generated, open it in VS 2019 and compile everything.
 
#### Linux (using make)

In the project root folder run following commands:

    mkdir build && cd build
    cmake ..
    make
    ./Ccraft
    
### Libraries used in Ccraft

* [GLFW](https://github.com/glfw/glfw) - Window creation and management
* [glad](https://github.com/Dav1dde/glad) - Loading modern OpenGL functions
* [cglm](https://github.com/recp/cglm) - Very cool maths library
* [SQLite](https://www.sqlite.org/index.html) - Writing and reading to/from database
* [stb_image](https://github.com/nothings/stb) - Loading .png files
* [FastNoise](https://github.com/Auburn/FastNoise) - Voronoi and Simplex noises for world generation
* [ini](https://github.com/rxi/ini) - Parsing .ini fonfuguration files

### Some implementation details

I got inspired by fogleman's [Craft](https://github.com/fogleman/Craft), but my development process bent over visual features, not gameplay ones.

Ccraft doesn't support multiplayer, but features rendering to textures, depth of field, motion blur, texture filtering, 
cubemap textures, array textures, loading setting from file rather than storing them as #defines, and different biomes.

#### Rendering

To support effects such as depth of field and motion blur, the deferred rendering must be used (rendering to texture).
For that, the separate framebuffer is created, and 4 textures are attached to it: texture for game image (1), texture for ui (2),
texture for game image with depth of field effect applied (3), texture for game depth (4).

The render steps are:

- The world is rendered to texture for game image (1), and the depth data goes to depth texture (4);
- The ui elements (outline of a block you look on, chosshair etc.) are rendered to texture for ui elements (3);
- The textures for game image (1) and depth texture (4) are being read in depth of field shader, and using that information the depth of 
field shader applies its effect, rendering result to the texture (3);
- All remaining effects are applied to the texture (3), ui (2) is added on top, and all that goes onto your screen.

[The ambient occlusion is implemented as described here!](https://0fps.net/2013/07/03/ambient-occlusion-for-minecraft-like-worlds)

#### Chunk system

The world is split into chunks. The size of a chunk can be changed in settings, the default is 32x256x32. 

The map has worker threads that are loading and updating chunks (generate terrain and mesh). Every frame main thread tries to 
find chunks to load/update and sends them to worker threads, which do all the hard work.

To load chunk means to generate its terrain and mesh;

To update chunk means to regenerate mesh of existing (loaded) chunk. Chunks need update when the block is 
placed/deleted - instead of trying to modify chunk's vertex buffer, it just regenerates. If the block is 
placed/deleted on the edge of the chunk, some neighbour chunks needs regeneration, too.

Main thread actually does a bit more: it loads chunks (without involving worker threads) that are very close to the player
(so there's ground below you, that's always good), and it deletes chunks that are too far from the player.

#### Chunk's neighbours

The chunk upon mesh generation needs to know information about blocks that are actually not a part of this chunk.
So, every chunk stores 1 additional layer of blocks in every direction. During terrain generation these additional 
layers are filled, and then this information is used in mesh generation.

But this neighbour information can be outdated (if some blocks have been placed/removed is neighbour chunks before),
so before mesh generation every chunk loads all block changes from the database.

#### Saving changes to the world and player's state

SQLlite database is used for it. The database has 3 tables, "blocks", "map_info", "player_info".

All block changes are stored in "blocks" table. The format is "chunk_x, chunk_z, block_x, block_y, block_z, block_type".
if the block change is on the edge of some chunk, additional database entries are created, so neighbour chunks, when
getting blocks from database, can see this shange. 

For example, when the diamond block is placed to coordinate (0, 60, 0), 4 database entries will be created, because 3
additional chunks store this block information besides chunk (0, 0). Here CHUNK_WIDTH == 32 is assumed:

| chunk_x | chunk_z | block_x | block_y | block_z | block_type    |
|---------|---------|---------|---------|---------|---------------|
| 0       | 0       | 0       | 60      | 0       | BLOCK_DIAMOND |
| -1      | 0       | 32      | 60      | 0       | BLOCK_DIAMOND |
| 0       | -1      | 0       | 60      | 32      | BLOCK_DIAMOND |
| -1      | -1      | 32      | 60      | 32      | BLOCK_DIAMOND |

"map_info" stores 4 values: current map time, map seed, and chunk width and height that were used when the map was created.
Storing width and height allows to compare it to the current CHUNK_WIDTH and CHUNK_HEIGHT during load (so to check the compatability).

"player_info" stores 3 coordinates for player position, camera's pitch and yaw, and current build block (the block player has in hands). Internally, player's 
hand is also just a block, but you can't build with it.

#### Terrain generation

The terrain is generated using different kinds of noise. For choosing biome, the voronoi noise is used, and for terrain height it's
simplex noise. 

If you just try to use different noise setting for different biomes, you will get non-smooth biome borders. So, the bilinear interpolation
is used to smooth out these borders: Every chunk firstly generates height only for every 8th block using simplex noise, and then the inteprolation
gets the remaining heighs.

For example, if we want to load chunk (0, 0) and CHUNK_WIDTH == 32, then:

- We need to generate blocks spanning from -1 to 32 inclisive in directions XZ (Chunk blocks are \[0, 31\], and neighbour information are
layers -1 and 32)

- For that, we generate firstly only generate height of these blocks: (-8, -8), (-8, 0), ..., (-8, 32), (-8, 40), (0, -8), (0, 0), ...

- Then all remaining heigths are interpolated from those that were generated in previous step. For example, to generate height for block (-1, -1)
we use these 4 blocks: (-8, -8), (-8, 0), (0, -8), (0, 0); to generate height for block (5, 13) we use these 4 blocks: (0, 8), (0, 16), (8, 8), (8, 16), etc.

#### Collision testing

Collision testing is implemented using axis-aligned bounding boxes (aabb). Every frame motion vector is generated (how much in each direction player should move),
and then for each of 3 coordinates the motion vector's component is added to player's position and collision with all surrounding blocks is checked. If there's a collision, just pull out player from the block player collides with.

If the motion vector magnitude is comparable to the size of a block, the cossision detection may not actually detect anything:
imagine a 1-block-width wall and player with huge speed heading into the wall: the player will just get through. 

To counter this, the "adding motion vector to player's position" is done in a loop, where the actual value added is much less than size
of a block.

