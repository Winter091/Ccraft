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
float DOF_SPEED                = 5.0;
int   FOV                      = 65;
int   FOV_ZOOM                 = 20;
float GAMMA                    = 1.5f;
float SATURATION               = 1.2f;

// [WINDOW] (default values)
const char* WINDOW_TITLE  = "SND Corp - Ccraft";
int         WINDOW_WIDTH  = 1280;
int         WINDOW_HEIGHT = 720;
int         FULLSCREEN    = 0;
int         VSYNC         = 0;

// [GAMEPLAY] (default values)
const char* MAP_NAME           = "default_map.db";
int         USE_MAP            = 1;
float       MOUSE_SENS         = 0.1f;
int         BLOCK_BREAK_RADIUS = 5;
int         DAY_LENGTH         = 1200;
int         DISABLE_TIME_FLOW  = 0;
float       DAY_LIGHT          = 0.85f;
float       EVENING_LIGHT      = 0.5f;
float       NIGHT_LIGHT        = 0.15f;

// [CORE] (default values)
int   CHUNK_WIDTH  = 32;
int   CHUNK_HEIGHT = 256;
float BLOCK_SIZE   = 0.1f;

// [PHYSICS] (default values)
float MAX_MOVE_SPEED          = 5.6f;
float MAX_MOVE_SPEED_SNEAK    = 1.3f;
float MAX_SWIM_SPEED          = 3.0f;
float MAX_FALL_SPEED          = 100.0f;
float MAX_DIVE_SPEED          = 4.0f;
float MAX_EMEGRE_SPEED        = 6.0f;
float ACCELERATION_HORIZONTAL = 40.0f;
float DECELERATION_HORIZONTAL = 50.0f;
float DECELERATION_VERTICAL   = 10.0f;
float JUMP_POWER              = 8.2f;
float GRAVITY                 = 25.0f;

// Internal values, cannot be set using ini file
int   OPENGL_VERSION_MAJOR_REQUIRED = 3;
int   OPENGL_VERSION_MINOR_REQUIRED = 3;
float DAY_TO_EVN_START              = 0.3f;
float EVN_TO_NIGHT_START            = 0.4f;
float NIGHT_START                   = 0.45f;
float NIGHT_TO_DAY_START            = 0.85f;
float CHUNK_SIZE                    = 32 * 0.1f;
int   CHUNK_LOAD_RADIUS             = 16 + 2;
int   CHUNK_UNLOAD_RADIUS           = 16 + 5;

ini_t* cfg = NULL;

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
    "title      = SND Corp - Ccraft\n"
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
    "fov      = 65\n"
    "fov_zoom = 20\n\n"

    "gamma      = 1.5\n"
    "saturation = 1.2\n\n"

    "[GAMEPLAY]\n"
    "map_name = default_map.db\n"
    "; You can disable loading and saving from/to map file;\n"
    "; The world seed will always be 1337 and all changes\n"
    "; to the world won't be remembered\n"
    "use_map = 1\n\n"

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
    "; Should be a multiple of 8 (8, 16, 24, 32 etc),\n"
    "; otherwise the gamme will crash/break.\n"
    "; Also, game and map are incompatible if\n" 
    "; widths are different. Reducing chunk width\n"
    "; is a great performance boost on a slower\n"
    "; proccessors\n"
    "chunk_width = 32\n\n"

    "; Probably you don't want to change height,\n"
    "; it will completely break world generation\n"
    "; and maybe something else\n"
    "chunk_height = 256\n\n"

    "; Width of one block in GPU memory. If it's\n"
    "; too small, you will experience floating-point\n" 
    "; errors everywhere, if it's too high you will\n"
    "; start to experience there errors not far from spawn\n"
    "block_size = 0.1\n\n"

    "[PHYSICS]\n"
    "; Everything except gravity is in blocks per second\n"
    "max_move_speed          = 5.6\n"
    "max_move_speed_sneak    = 1.3\n"
    "max_swim_speed          = 3.0\n"
    "max_fall_speed          = 100.0\n"
    "max_dive_speed          = 4.0\n"
    "max_emerge_speed        = 6.0\n"
    "acceleration_horizontal = 40.0\n"
    "deceleration_horizontal = 50.0\n"
    "deceleration_vertical   = 10.0\n"
    "jump_power              = 8.2\n"
    "gravity                 = 25.0\n";

    printf("Creating new '%s' file...\n", "config.ini");
    fprintf(f, "%s", content);
    fclose(f);
}

// Translate physics values from "some units per second"
// to "blocks per second"
static void normalize_player_physics()
{
    MAX_MOVE_SPEED          *= BLOCK_SIZE;
    MAX_MOVE_SPEED_SNEAK    *= BLOCK_SIZE;
    MAX_SWIM_SPEED          *= BLOCK_SIZE;
    MAX_FALL_SPEED          *= BLOCK_SIZE;
    MAX_DIVE_SPEED          *= BLOCK_SIZE;
    MAX_EMEGRE_SPEED        *= BLOCK_SIZE;
    ACCELERATION_HORIZONTAL *= BLOCK_SIZE;
    DECELERATION_HORIZONTAL *= BLOCK_SIZE;
    DECELERATION_VERTICAL   *= BLOCK_SIZE;
    JUMP_POWER              *= BLOCK_SIZE;
    GRAVITY                 *= BLOCK_SIZE;
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

void config_load()
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
    try_load("GAMEPLAY", "use_map", "%d", &USE_MAP);
    try_load("GAMEPLAY", "mouse_sens", "%f", &MOUSE_SENS);
    try_load("GAMEPLAY", "block_break_radius", "%d", &BLOCK_BREAK_RADIUS);
    try_load("GAMEPLAY", "day_length", "%d", &DAY_LENGTH);
    try_load("GAMEPLAY", "disable_time_flow", "%d", &DISABLE_TIME_FLOW);
    try_load("GAMEPLAY", "day_light", "%f", &DAY_LIGHT);
    try_load("GAMEPLAY", "evening_light", "%f", &EVENING_LIGHT);
    try_load("GAMEPLAY", "night_light", "%f", &NIGHT_LIGHT);

    try_load("CORE", "chunk_width", "%d", &CHUNK_WIDTH);
    try_load("CORE", "chunk_height", "%d", &CHUNK_HEIGHT);
    try_load("CORE", "block_size", "%f", &BLOCK_SIZE);

    try_load("PHYSICS", "max_move_speed", "%f", &MAX_MOVE_SPEED);
    try_load("PHYSICS", "max_move_speed_sneak", "%f", &MAX_MOVE_SPEED_SNEAK);
    try_load("PHYSICS", "max_swim_speed", "%f", &MAX_SWIM_SPEED);
    try_load("PHYSICS", "max_fall_speed", "%f", &MAX_FALL_SPEED);
    try_load("PHYSICS", "max_dive_speed", "%f", &MAX_DIVE_SPEED);
    try_load("PHYSICS", "max_emerge_speed", "%f", &MAX_EMEGRE_SPEED);
    try_load("PHYSICS", "acceleration_horizontal", "%f", &ACCELERATION_HORIZONTAL);
    try_load("PHYSICS", "deceleration_horizontal", "%f", &DECELERATION_HORIZONTAL);
    try_load("PHYSICS", "deceleration_vertical", "%f", &DECELERATION_VERTICAL);
    try_load("PHYSICS", "jump_power", "%f", &JUMP_POWER);
    try_load("PHYSICS", "gravity", "%f", &GRAVITY);

    normalize_player_physics();

    CHUNK_SIZE = CHUNK_WIDTH * BLOCK_SIZE;
    CHUNK_LOAD_RADIUS = CHUNK_RENDER_RADIUS + 2;
    CHUNK_UNLOAD_RADIUS = CHUNK_RENDER_RADIUS + 5;

    printf("Loaded everything.\n");
}