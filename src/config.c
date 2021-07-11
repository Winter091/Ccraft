#include "config.h"

#include "stdio.h"
#include "string.h"

#include "ini.h"

// [GRAPHICS] (default values)
int   CHUNK_RENDER_RADIUS      = 16;
int   ANISOTROPIC_FILTER_LEVEL = 16;
int   MOTION_BLUR_ENABLED      = 1;
float MOTION_BLUR_STRENGTH     = 0.0005f;
int   MOTION_BLUR_SAMPLES      = 7;
int   DOF_ENABLED              = 1;
int   DOF_SMOOTH               = 1;
float DOF_MAX_BLUR             = 0.023f;
float DOF_APERTURE             = 0.3505f;
float DOF_SPEED                = 5.0f;
int   FOV                      = 65;
int   FOV_ZOOM                 = 20;
float GAMMA                    = 1.5f;
float SATURATION               = 1.2f;

// [WINDOW] (default values)
const char* WINDOW_TITLE  = "Ccraft";
int         WINDOW_WIDTH  = 1280;
int         WINDOW_HEIGHT = 720;
int         FULLSCREEN    = 0;
int         VSYNC         = 0;

// [GAMEPLAY] (default values)
const char* MAP_NAME           = "default_map.db";
float       MOUSE_SENS         = 0.1f;
int         BLOCK_BREAK_RADIUS = 5;
int         DAY_LENGTH         = 1200;
int         DISABLE_TIME_FLOW  = 0;
float       DAY_LIGHT          = 0.85f;
float       EVENING_LIGHT      = 0.5f;
float       NIGHT_LIGHT        = 0.15f;

// [CORE] (default values)
int   NUM_WORKERS  = 0;
int   CHUNK_WIDTH  = 32;
int   CHUNK_HEIGHT = 256;
float BLOCK_SIZE   = 0.1f;

// [PHYSICS] (default values)
float MAX_RUN_SPEED           = 8.0f;
float MAX_MOVE_SPEED          = 5.6f;
float MAX_SNEAK_SPEED         = 1.3f;
float MAX_SWIM_SPEED          = 3.0f;
float MAX_FALL_SPEED          = 100.0f;
float MAX_DIVE_SPEED          = 4.0f;
float MAX_EMERGE_SPEED        = 6.0f;
float JUMP_POWER              = 8.2f;

float ACCELERATION_WATER_EMERGE = 10.0f;
float ACCELERATION_HORIZONTAL   = 40.0f;
float DECELERATION_HORIZONTAL   = 50.0f;
float GRAVITY                   = 25.0f;
float GRAVITY_WATER             = 10.0f;

// Internal values, cannot be set using ini file
int   OPENGL_VERSION_MAJOR_REQUIRED = 3;
int   OPENGL_VERSION_MINOR_REQUIRED = 3;
float DAY_TO_EVN_START              = 0.3f;
float EVN_TO_NIGHT_START            = 0.4f;
float NIGHT_START                   = 0.45f;
float NIGHT_TO_DAY_START            = 0.85f;
float CHUNK_SIZE                    = 32 * 0.1f;
int   CHUNK_LOAD_RADIUS             = 16 + 2;
int   CHUNK_LOAD_RADIUS2            = (16 + 2) * (16 + 2);
int   CHUNK_UNLOAD_RADIUS           = 16 + 5;
int   CHUNK_UNLOAD_RADIUS2          = (16 + 5) * (16 + 5);
int   BLOCK_BREAK_RADIUS2           = 5 * 5;
int   CHUNK_RENDER_RADIUS2          = 16 * 16;
int   CHUNK_WIDTH_REAL              = 32 + 2;
int   CHUNK_HEIGHT_REAL             = 256 + 2;

static ini_t* cfg;

static void create_default_cfg_file()
{
    FILE* f = fopen("config.ini", "w");
    if (!f)
    {
        fprintf(stderr, "Failed to create new '%s' file\n", "config.ini");
        return;
    }

    const char* content = 
    "; Configuration file for Ccraft\n"
    "; If you've messed something up, you can\n"
    "; delete this file and launch the game.\n"
    "; Default config file will be created.\n\n"

    "[WINDOW]\n"
    "title      = Ccraft\n"
    "width      = 1280\n"
    "height     = 720\n"
    "fullscreen = 0\n"
    "vsync      = 0 ; Locks FPS at monitor max refresh rate\n\n"

    "[GRAPHICS]\n"
    "; High performance hit\n"
    "chunk_render_radius = 16\n\n"

    "; Very low performance hit, huge\n"
    "; image quality boost\n"
    "anisotropic_filter_level = 16\n\n"

    "; Low performance hit if amount\n"
    "; of samples is moderate\n"
    "motion_blur_enabled  = 1\n"
    "motion_blur_strength = 0.0005\n"
    "motion_blur_samples  = 7\n\n"

    "; Medium performance hit if not a smooth\n"
    "; variant; Smooth DoF is much more expensive\n"
    "depth_of_field_enabled  = 1\n"
    "depth_of_field_smooth   = 1\n"
    "depth_of_field_max_blur = 0.023\n"
    "depth_of_field_aperture = 0.3505\n"
    "depth_of_field_speed    = 5.0 ; Affects only smooth depth of field\n\n"

    "; Field of view\n"
    "fov      = 75\n"
    "fov_zoom = 20\n\n"

    "gamma      = 1.5\n"
    "saturation = 1.2\n\n"

    "[GAMEPLAY]\n"
    "map_name = default_map.db\n"
    "mouse_sens = 0.1\n\n"

    "; Furthest distance (in blocks) for player to reach\n"
    "block_break_radius = 5\n\n"

    "; Full day length in seconds\n"
    "day_length = 1200\n\n"

    "; Disable time, make it always day\n"
    "disable_time_flow = 0\n\n"

    "; Light multipliers for different times of day\n"
    "day_light     = 0.85\n"
    "evening_light = 0.5\n"
    "night_light   = 0.15\n\n"

    "[CORE]\n"
    "; Number of threads to use to load chunks. Set to 0\n"
    "; to determine automatically\n"
    "num_workers = 0\n\n"

    "; Probably you shouldn't even dare to touch it\n"
    "chunk_width = 32\n\n"

    "; And also don't touch that!\n"
    "chunk_height = 256\n\n"

    "; Width of one block in GPU memory. If it's\n"
    "; too small, you will experience floating-point\n" 
    "; errors everywhere, if it's too high you will\n"
    "; start to experience there errors not far from spawn\n"
    "block_size = 0.1\n\n"

    "[PHYSICS]\n"
    "; Blocks per second\n"
    "max_run_speed    = 5.612\n"
    "max_move_speed   = 4.317\n"
    "max_sneak_speed  = 1.31\n"
    "max_swim_speed   = 3.5\n"
    "max_fall_speed   = 57.46\n"
    "max_dive_speed   = 8.0\n"
    "max_emerge_speed = 10.0\n"
    "jump_power       = 8.3\n"

    "; Blocks per second^2\n"
    "acceleration_water_emerge = 20.0\n"
    "acceleration_horizontal   = 40.0\n"
    "deceleration_horizontal   = 40.0\n"
    "gravity                   = 27.44\n"
    "gravity_water             = 9.14\n";

    printf("Creating new '%s' file...\n", "config.ini");
    fprintf(f, "%s", content);
    fclose(f);
}

static void normalize_player_physics()
{
    MAX_RUN_SPEED           *= BLOCK_SIZE;
    MAX_MOVE_SPEED          *= BLOCK_SIZE;
    MAX_SNEAK_SPEED         *= BLOCK_SIZE;
    MAX_SWIM_SPEED          *= BLOCK_SIZE;
    MAX_FALL_SPEED          *= BLOCK_SIZE;
    MAX_DIVE_SPEED          *= BLOCK_SIZE;
    MAX_EMERGE_SPEED        *= BLOCK_SIZE;
    JUMP_POWER              *= BLOCK_SIZE;

    ACCELERATION_WATER_EMERGE *= BLOCK_SIZE;
    ACCELERATION_HORIZONTAL   *= BLOCK_SIZE;
    DECELERATION_HORIZONTAL   *= BLOCK_SIZE;
    GRAVITY                   *= BLOCK_SIZE;
    GRAVITY_WATER             *= BLOCK_SIZE;
}

static void try_load(const char* section, const char* key, const char* fmt, void* dst)
{
    if (!ini_sget(cfg, section, key, fmt, dst))
    {
        fprintf(stderr, "\nFailed to load '%s' parameter!\n", key);
        fprintf(stderr, "Using default value: ");

        if (!fmt)
            fprintf(stderr, "%s\n", *(const char**)dst);
        else if (!strcmp(fmt, "%d"))
            fprintf(stderr, "%d\n", *(int*)dst);
        else
            fprintf(stderr, "%f\n", *(float*)dst);
    }
}

void config_init()
{
    cfg = ini_load("config.ini");
    if (!cfg)
    {
        fprintf(stderr, "Config file '%s' was not found.\n", "config.ini");
        fprintf(stderr, "Using default settings.\n");

        create_default_cfg_file();
        normalize_player_physics();
        return;
    }
    else
    {
        printf("Loading settings from '%s'...\n", "config.ini");
    }

    try_load("GRAPHICS", "chunk_render_radius", "%d", &CHUNK_RENDER_RADIUS);
    try_load("GRAPHICS", "anisotropic_filter_level", "%d", &ANISOTROPIC_FILTER_LEVEL);
    try_load("GRAPHICS", "motion_blur_enabled", "%d", &MOTION_BLUR_ENABLED);
    try_load("GRAPHICS", "motion_blur_strength", "%f", &MOTION_BLUR_STRENGTH);
    try_load("GRAPHICS", "motion_blur_samples", "%d", &MOTION_BLUR_SAMPLES);
    try_load("GRAPHICS", "depth_of_field_enabled", "%d", &DOF_ENABLED);
    try_load("GRAPHICS", "depth_of_field_smooth", "%d", &DOF_SMOOTH);
    try_load("GRAPHICS", "depth_of_field_max_blur", "%f", &DOF_MAX_BLUR);
    try_load("GRAPHICS", "depth_of_field_aperture", "%f", &DOF_APERTURE);
    try_load("GRAPHICS", "depth_of_field_speed", "%f", &DOF_SPEED);
    try_load("GRAPHICS", "fov", "%d", &FOV);
    try_load("GRAPHICS", "fov_zoom", "%d", &FOV_ZOOM);
    try_load("GRAPHICS", "gamma", "%f", &GAMMA);
    try_load("GRAPHICS", "saturation", "%f", &SATURATION);

    try_load("WINDOW", "title", NULL, &WINDOW_TITLE);
    try_load("WINDOW", "width", "%d", &WINDOW_WIDTH);
    try_load("WINDOW", "height", "%d", &WINDOW_HEIGHT);
    try_load("WINDOW", "fullscreen", "%d", &FULLSCREEN);
    try_load("WINDOW", "vsync", "%d", &VSYNC);

    try_load("GAMEPLAY", "map_name", NULL, &MAP_NAME);
    try_load("GAMEPLAY", "mouse_sens", "%f", &MOUSE_SENS);
    try_load("GAMEPLAY", "block_break_radius", "%d", &BLOCK_BREAK_RADIUS);
    try_load("GAMEPLAY", "day_length", "%d", &DAY_LENGTH);
    try_load("GAMEPLAY", "disable_time_flow", "%d", &DISABLE_TIME_FLOW);
    try_load("GAMEPLAY", "day_light", "%f", &DAY_LIGHT);
    try_load("GAMEPLAY", "evening_light", "%f", &EVENING_LIGHT);
    try_load("GAMEPLAY", "night_light", "%f", &NIGHT_LIGHT);

    try_load("CORE", "num_workers", "%d", &NUM_WORKERS);
    try_load("CORE", "chunk_width", "%d", &CHUNK_WIDTH);
    try_load("CORE", "chunk_height", "%d", &CHUNK_HEIGHT);
    try_load("CORE", "block_size", "%f", &BLOCK_SIZE);

    try_load("PHYSICS", "max_run_speed", "%f", &MAX_RUN_SPEED);
    try_load("PHYSICS", "max_move_speed", "%f", &MAX_MOVE_SPEED);
    try_load("PHYSICS", "max_sneak_speed", "%f", &MAX_SNEAK_SPEED);
    try_load("PHYSICS", "max_swim_speed", "%f", &MAX_SWIM_SPEED);
    try_load("PHYSICS", "max_fall_speed", "%f", &MAX_FALL_SPEED);
    try_load("PHYSICS", "max_dive_speed", "%f", &MAX_DIVE_SPEED);
    try_load("PHYSICS", "max_emerge_speed", "%f", &MAX_EMERGE_SPEED);
    try_load("PHYSICS", "jump_power", "%f", &JUMP_POWER);

    try_load("PHYSICS", "acceleration_water_emerge", "%f", &ACCELERATION_WATER_EMERGE);
    try_load("PHYSICS", "acceleration_horizontal", "%f", &ACCELERATION_HORIZONTAL);
    try_load("PHYSICS", "deceleration_horizontal", "%f", &DECELERATION_HORIZONTAL);
    try_load("PHYSICS", "gravity", "%f", &GRAVITY);
    try_load("PHYSICS", "gravity_water", "%f", &GRAVITY_WATER);

    normalize_player_physics();

    CHUNK_SIZE          = (float)CHUNK_WIDTH * BLOCK_SIZE;
    CHUNK_LOAD_RADIUS   = CHUNK_RENDER_RADIUS + 2;
    CHUNK_UNLOAD_RADIUS = CHUNK_RENDER_RADIUS + 5;

    BLOCK_BREAK_RADIUS2  = BLOCK_BREAK_RADIUS  * BLOCK_BREAK_RADIUS;
    CHUNK_RENDER_RADIUS2 = CHUNK_RENDER_RADIUS * CHUNK_RENDER_RADIUS;
    CHUNK_LOAD_RADIUS2   = CHUNK_LOAD_RADIUS   * CHUNK_LOAD_RADIUS;
    CHUNK_UNLOAD_RADIUS2 = CHUNK_UNLOAD_RADIUS * CHUNK_UNLOAD_RADIUS;

    CHUNK_WIDTH_REAL  = CHUNK_WIDTH + 2;
    CHUNK_HEIGHT_REAL = CHUNK_HEIGHT + 2;

    printf("Loaded everything.\n");
}

void config_free()
{
    if (cfg)
    {
        ini_free(cfg);
        cfg = NULL;
    }
}