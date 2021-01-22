// Use this version of OpenGL
#define OPENGL_VERSION_MAJOR_REQUIRED 3
#define OPENGL_VERSION_MINOR_REQUIRED 3

// ================== Graphics ==================
#define CHUNK_RENDER_RADIUS 16

// Anisotropic filtering. Accepted values: 1 2 4 8 16
#define ANISOTROPIC_FILTERING_LEVEL 16

// Motion blur effect
#define MOTION_BLUR_ENABLED 1
#define MOTION_BLUR_STRENGTH 0.0005f
#define MOTION_BLUR_SAMPLES 7

// Depth of field effect
#define DOF_ENABLED 1
#define DOF_SMOOTH 1  // Smooth DoF is very expensive but looks good
#define DOF_MAX_BLUR 0.023f
#define DOF_APERTURE 0.3505f
#define DOF_SPEED 5.0f

// gamma should be 2.2, but it looks very washed out
#define GAMMA_CORRECTION 1.5f
#define SATURATION 1.2f

// ============== Window settings ===============
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "Ccraft Window"
#define FULLSCREEN 0
#define VSYNC 0

// ============= Gameplay settings ==============
#define CURRENT_MAP "maps/test_map.db"
#define USE_DATABASE 1

// full day length in seconds
#define DAY_LENGTH 1200
#define DISABLE_TIME_FLOW 0

// full day range is [0.0 - 1.0)
#define DAY_TO_EVN_START 0.3f
#define EVN_TO_NIGHT_START 0.4f
#define NIGHT_START 0.45f
#define NIGHT_TO_DAY_START 0.85f

#define DAY_LIGHT 0.85f
#define EVENING_LIGHT 0.5f
#define NIGHT_LIGHT 0.15f

#define FOV 65
#define FOV_ZOOM 20

#define MOUSE_SENS 0.1f

// ============ Core game settings ==============

// Chunk width should be a multiple of 8, otherwise
// world generation will completely break
#define CHUNK_WIDTH 32

// Changing height will also break world generation
#define CHUNK_HEIGHT 256

#define BLOCK_SIZE 0.1f
#define CHUNK_SIZE (float)(CHUNK_WIDTH * BLOCK_SIZE)

#define BLOCK_BREAK_RADIUS 5
#define CHUNK_LOAD_RADIUS (CHUNK_RENDER_RADIUS + 2)
#define CHUNK_UNLOAD_RADIUS (CHUNK_RENDER_RADIUS + 5)

// ============== Player Physics ================
#define MAX_MOVE_SPEED            5.6f * BLOCK_SIZE
#define MAX_MOVE_SPEED_SNEAK      1.3f * BLOCK_SIZE
#define MAX_MOVE_SPEED_WATER      3.0f * BLOCK_SIZE
#define MAX_FALL_SPEED          100.0f * BLOCK_SIZE
#define MAX_DIVE_SPEED_WATER      4.0f * BLOCK_SIZE
#define MAX_EMEGRE_SPEED_WATER    6.0f * BLOCK_SIZE
#define ACCELERATION_HORIZONTAL  40.0f * BLOCK_SIZE
#define DECELERATION_HORIZONTAL  50.0f * BLOCK_SIZE
#define DECELERATION_VERTICAL    10.0f * BLOCK_SIZE
#define JUMP_POWER                8.2f * BLOCK_SIZE
#define GRAVITY                   2.7f