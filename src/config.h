#ifndef CONFIG_H_
#define CONFIG_H_

// [GRAPHICS]
extern int   CHUNK_RENDER_RADIUS;
extern int   ANISOTROPIC_FILTER_LEVEL;
extern int   MOTION_BLUR_ENABLED;
extern float MOTION_BLUR_STRENGTH;
extern int   MOTION_BLUR_SAMPLES;
extern int   DOF_ENABLED;
extern int   DOF_SMOOTH;
extern float DOF_MAX_BLUR;
extern float DOF_APERTURE;
extern float DOF_SPEED;
extern int   FOV;
extern int   FOV_ZOOM;
extern float GAMMA;
extern float SATURATION;

// [WINDOW]
extern const char* WINDOW_TITLE;
extern int         WINDOW_WIDTH;
extern int         WINDOW_HEIGHT;
extern int         FULLSCREEN;
extern int         VSYNC;

// [GAMEPLAY]
extern const char* MAP_NAME;
extern float       MOUSE_SENS;
extern int         BLOCK_BREAK_RADIUS;
extern int         DAY_LENGTH;
extern int         DISABLE_TIME_FLOW;
extern float       DAY_LIGHT;
extern float       EVENING_LIGHT;
extern float       NIGHT_LIGHT;

// [CORE]
extern int CHUNK_WIDTH;
extern int CHUNK_HEIGHT;
extern float BLOCK_SIZE;

// [PHYSICS]
extern float MAX_MOVE_SPEED;
extern float MAX_MOVE_SPEED_SNEAK;
extern float MAX_SWIM_SPEED;
extern float MAX_FALL_SPEED;
extern float MAX_DIVE_SPEED;
extern float MAX_EMEGRE_SPEED;
extern float ACCELERATION_HORIZONTAL;
extern float DECELERATION_HORIZONTAL;
extern float DECELERATION_VERTICAL;
extern float JUMP_POWER;
extern float GRAVITY;

// Internal values, cannot be set using ini file
extern int   OPENGL_VERSION_MAJOR_REQUIRED;
extern int   OPENGL_VERSION_MINOR_REQUIRED;
extern float DAY_TO_EVN_START;
extern float EVN_TO_NIGHT_START;
extern float NIGHT_START;
extern float NIGHT_TO_DAY_START;
extern float CHUNK_SIZE;
extern int   CHUNK_LOAD_RADIUS;
extern int   CHUNK_LOAD_RADIUS2;
extern int   CHUNK_UNLOAD_RADIUS;
extern int   CHUNK_UNLOAD_RADIUS2;
extern int   BLOCK_BREAK_RADIUS2;
extern int   CHUNK_RENDER_RADIUS2;
extern int   CHUNK_XZ_REAL;
extern int   CHUNK_XY_REAL;

void config_load();

#endif
