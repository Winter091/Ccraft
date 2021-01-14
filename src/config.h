// Use this version of OpenGL
#define OPENGL_VERSION_MAJOR_REQUIRED 3
#define OPENGL_VERSION_MINOR_REQUIRED 3

// ================== Graphics ==================

#define CHUNK_RENDER_RADIUS 16

// Anisotropic filtering. Accepted values: 1 2 4 8 16
#define ANISOTROPIC_FILTERING_LEVEL 16

// Motion blur effect
#define MOTION_BLUR_ENABLED 1
#define MOTION_BLUR_STRENGTH 0.001f
#define MOTION_BLUR_SAMPLES 7

// Depth of field effect
#define DOF_ENABLED 0
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

#define DISABLE_TIME_FLOW 0

// full day length in seconds
#define DAY_LENGTH 1200

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

// ============ Core game settings ==============
#define CHUNK_WIDTH 32
#define CHUNK_HEIGHT 256
#define BLOCK_SIZE 0.5f
#define CHUNK_SIZE (float)(CHUNK_WIDTH * BLOCK_SIZE)

#define BLOCK_BREAK_RADIUS 6
#define CHUNK_LOAD_RADIUS (CHUNK_RENDER_RADIUS + 2)
#define CHUNK_UNLOAD_RADIUS (CHUNK_RENDER_RADIUS + 5)
