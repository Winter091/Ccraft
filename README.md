# Ccraft

It's a minecraft-inspired game written in C using modern OpenGL.

There are not a lot of features, currently there is:

* Infinite world generation
* Biomes, each with unique heightmap
* Player physics (on land and in water)
* Saving and loading map from database file
* Saving and loading player state from database file
* Building & destroying blocks
* Depth of field effect (smooth / non-smooth)
* Motion blur effect
* Day/night cycles
* Using configuration file for loading all settings

### Screenshots

![1](https://github.com/Winter091/Ccraft/blob/main/screenshots/biomes.png)
![2](https://github.com/Winter091/Ccraft/blob/main/screenshots/morning.png)
![3](https://github.com/Winter091/Ccraft/blob/main/screenshots/forest.png)
![4](https://github.com/Winter091/Ccraft/blob/main/screenshots/mountains.png)
![5](https://github.com/Winter091/Ccraft/blob/main/screenshots/night-2.png)

### Controls

* WASD               - Move player
* Tab                - Toggle fly mode
* Shift              - Slow movement / fly up
* Ctrl               - Fly down
* Page up/down       - Increase/decrease fly camera speed
* C                  - Zoom camera
* Left mouse button  - Destroy block
* Right mouse button - Place block
* Mouse scroll wheel - Change build block

### How to play

Binaries for windows 10 x64 and Linux x64 are available in the [releases](https://github.com/Winter091/Ccraft/releases) tab.

You can change game settings by modifying `config.ini` file.

### How to compile

You may need to install some additional dependencies, CMake will tell you about them.

#### Windows

Download and install [CMake](https://cmake.org/download/).

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

Install CMake:

    sudo apt-get install cmake
    
Then in the project root folder run following commands:

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

#### Rendering and graphics

A couple of rendering optimizations are implemented, such as rendering only exposed faces, using face culling not to draw back faces of triangles,
and frustum culling, which is not drawing chunks that are not visible.

Each chunk has 2 vertex buffers, one for water blocks, the other for everything else. Using separate buffer for water is nessessary to ensure the right 
rendering order, so everything will be seen through transparent blocks.

Water has actual transparency, but there's also alpha testing (binary transparency). In shader the alpha value of fragment is checked, and if it's less than 0.5, 
the fragment is discarded. Alpha testing allows to draw transparent glass, flowers, grass and tree leaves.

When block in some chunk is changed, the chunk buffers are being completely rebuilt.

To implement post-processing visual effects, such as depth of field, motion blur, gamma correction and saturation tweak, 
separate framebuffer had to be used. So, the game is being rendered on a texture, then it's being rendered on yet another texture
and during that the depth of field is applied, and in the final shader pass all other effects are applied and this image is actually what the player (you)
sees. I really like the result, DoF and motion blur both look very good.

There's ambient occlusion effect, it's implemented as described on [this](https://0fps.net/2013/07/03/ambient-occlusion-for-minecraft-like-worlds/) page.

Anisotropic texture filtering is used for tilemap, if supported by hardware. Otherwise, the usual texture filtering is used.

Array texture is used for storing all 256 tiles, it's much better than using plain 2D texture and sample block textures from it. Array texture allows to use
texture filtering without artifact when adjacent tiles are blended during downsampling.

Sky is rendered using cubemap textures, there are 3 for different times of day: day, evening and night. They're blended together depending on
the current time of day. Also, the night texture (stars) is being rotated over time, thus creating beautiful night scene.

Sun and moon are just textures that are rendered on quads and also being rotated over time.

#### Terrain generation

The world is split into chunks. The size of a chunk can be changed in settings, but usually it's 16x256x16 or 32x256x32. Each frame one chunk is loaded,
if not every chunk in load distance is loaded yet. The chunk to load is determined by the distance to the player and the visibility for the player. So, 
the closest visible to the player chunk will be loaded; If there are no visible chunks to load, just the closest is loaded.

Height and biome can be obtained for every block using noise functions. For biomes the Voronoi noise is used, for getting height the Simplex noise is used.

Biomes with different noise settings introduce huge "cliffs" where they meet. To smooth out the bounds, 2d linear interpolation is used. At first, only some 
terrain heights are generated, specifically each 8th block gets height. All remaining heights are obtained using interpolation between four "corner" heights
that were generated in the first step. It helps not only to smooth out biome edges, but also improves performance, because noise function is used only for 
every 16th block in a chunk.

#### Saving world and player state

SQLlite database is used for it. The database has 3 tables, "blocks", "map_info", "player_info".

All block changes are stored in "blocks" table. The format is "chunk_x, chunk_z, block_x, block_y, block_z, block_type".

"map_info" stores 2 values, current map time and map seed.

"player_info" stores 3 coordinates for player position, camera's pitch and yaw, and current build block (the block player has in hands). Internally, player's 
hand is also just a block, but you can't access it in game.

#### Collision testing

Collision testing is implemented using axis-aligned bounding boxes (aabb). Every frame motion vector is generated (how much in each direction player should move),
and then for each of 3 coordinates the motion vectors' component is added to player's position and collisiong with all surrounding blocks is checked. If there's 
a collision, just pull out player from the block player collides with.



