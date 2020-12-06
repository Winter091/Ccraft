// use this version of OpenGL
#define OPENGL_VERSION_MAJOR_REQUIRED 3
#define OPENGL_VERSION_MINOR_REQUIRED 3

// Initial window settings
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "Ccraft Window"
#define FULLSCREEN 0
#define VSYNC 0

// game settings
#define FOV 65
#define CHUNK_WIDTH 32
#define CHUNK_HEIGHT 256
#define BLOCK_SIZE 0.1f
#define CHUNK_SIZE (float)(CHUNK_WIDTH * BLOCK_SIZE)

#define CHUNK_RENDER_RADIUS 10
#define CHUNK_LOAD_RADIUS (CHUNK_RENDER_RADIUS + 2)
#define CHUNK_UNLOAD_RADIUS (CHUNK_RENDER_RADIUS + 5)
#define MAP_FOLDER "test_map"

// block id's
#define BLOCK_AIR   0
#define BLOCK_GRASS 1
#define BLOCK_SAND  2
