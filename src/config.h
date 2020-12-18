// use this version of OpenGL
#define OPENGL_VERSION_MAJOR_REQUIRED 3
#define OPENGL_VERSION_MINOR_REQUIRED 3

// Graphics

// anisotropic filtering. Accepted values: 1 2 4 8 16
#define ANISOTROPIC_FILTERING_LEVEL 16

// MSAA antialiasing. Accepted values: 0, 2, 4, 8, 16
#define MSAA_LEVEL 4

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
#define BLOCK_SIZE 0.5f
#define CHUNK_SIZE (float)(CHUNK_WIDTH * BLOCK_SIZE)

#define BLOCK_BREAK_RADIUS 6
#define CHUNK_RENDER_RADIUS 16
#define CHUNK_LOAD_RADIUS (CHUNK_RENDER_RADIUS + 2)
#define CHUNK_UNLOAD_RADIUS (CHUNK_RENDER_RADIUS + 5)
#define CURRENT_MAP "maps/test_map.db"
