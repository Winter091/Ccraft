# Ccraft

It's a minecraft-inspired game written in C using modern OpenGL.

There are not a lot of features, currently featuring:

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

#### Windows (using MinGW32-make)

Download and install [CMake](https://cmake.org/download/) and [MinGW](https://sourceforge.net/projects/mingw-w64/).

Then in the project root folder run following commands:

    mkdir build && cd build
    cmake .. -G "MinGW Makefiles"
    mingw32-make
    Ccraft

#### Linux (using make)

Install CMake:

    sudo apt-get install cmake
    
Then in the project root folder run following commands:

    mkdir build && cd build
    cmake ..
    make
    ./Ccraft
    
### Libraries used in Ccraft

* **GLFW** - Window creation and management
* **GLAD** - Loading modern OpenGL functions
* **cglm** - Very cool maths library
* **sqlite** - Writing and reading to/from database
* **stb_image** - Loading .png files
* **FastNoise** - Voronoi and Simplex noises for world generation
* **ini** - Parsing .ini fonfuguration files
