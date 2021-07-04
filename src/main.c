#include "stdio.h"
#include "stdlib.h"

#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "cglm/cglm.h"

#include "config.h"
#include "shader.h"
#include "camera.h"
#include "window.h"
#include "texture.h"
#include "db.h"
#include "fastnoiselite.h"
#include "tinycthread.h"

// Print OpenGL warnings and errors
void opengl_debug_callback(
    GLenum source, GLenum type, GLuint id, GLenum severity, 
    GLsizei length, const GLchar* message, const void* userParam
)
{
    char* _source;
    char* _type;
    char* _severity;

    switch (source) {
        case GL_DEBUG_SOURCE_API_ARB:
        _source = "API";
        break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
        _source = "WINDOW SYSTEM";
        break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
        _source = "SHADER COMPILER";
        break;

        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
        _source = "THIRD PARTY";
        break;

        case GL_DEBUG_SOURCE_APPLICATION_ARB:
        _source = "APPLICATION";
        break;

        case GL_DEBUG_SOURCE_OTHER_ARB:
        _source = "UNKNOWN";
        break;

        default:
        _source = "UNKNOWN";
        break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR_ARB:
        _type = "ERROR";
        break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
        _type = "DEPRECATED BEHAVIOR";
        break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
        _type = "UDEFINED BEHAVIOR";
        break;

        case GL_DEBUG_TYPE_PORTABILITY_ARB:
        _type = "PORTABILITY";
        break;

        case GL_DEBUG_TYPE_PERFORMANCE_ARB:
        _type = "PERFORMANCE";
        break;

        case GL_DEBUG_TYPE_OTHER_ARB:
        _type = "OTHER";
        break;

        default:
        _type = "UNKNOWN";
        break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH_ARB:
        _severity = "HIGH";
        break;

        case GL_DEBUG_SEVERITY_MEDIUM_ARB:
        _severity = "MEDIUM";
        break;

        case GL_DEBUG_SEVERITY_LOW_ARB:
        _severity = "LOW";
        break;

        default:
        _severity = "UNKNOWN";
        break;
    }

    printf("%d: %s of %s severity, raised from %s: %s\n",
            id, _type, _severity, _source, message);
}

// Show fps in window title bar
static void print_fps()
{
    static double last_time = -1.0;
    if (last_time < 0)
    {
        last_time = glfwGetTime();
    }

    static int frames = 0;
    frames++;

    double curr_time = glfwGetTime();
    if (curr_time - last_time >= 1.0)
    {
        char title[128];
        sprintf(title, "%s - %d FPS", WINDOW_TITLE, frames);
        glfwSetWindowTitle(g_window->glfw, title);
        frames = 0;
        last_time = curr_time;
    }
}

// Get time between current and last frame
static double get_dt()
{
    static double last_time = -1.0;
    if (last_time < 0)
    {
        last_time = glfwGetTime();
    }

    double curr_time = glfwGetTime();
    double dt = curr_time - last_time;
    last_time = curr_time;

    return dt;
}

static float get_current_dof_depth(float dt)
{
    // glReadPixels is very slow :(
    
    static float curr_depth = 1.0f;

    // Read depth in the center of the screen 
    // and gradually move towards it
    float desired_depth;
    glReadPixels(g_window->width / 2, g_window->height / 2, 1, 1, 
                 GL_DEPTH_COMPONENT, GL_FLOAT, &desired_depth);
    
    curr_depth = glm_lerp(curr_depth, desired_depth, dt * DOF_SPEED);
    return curr_depth;
}

void update(Player* p, float dt)
{
    map_update(p->cam);
    player_update(p, dt);
}

void render_game(Player* p)
{
    framebuffer_use(g_window->fb, FBTYPE_TEXTURE);
    glClear(GL_DEPTH_BUFFER_BIT);

    framebuffer_use_texture(TEX_COLOR);
    {
        map_render_sky(p->cam);
        map_render_sun_moon(p->cam);
        map_render_chunks(p->cam);
    }

    framebuffer_use_texture(TEX_UI);
    {
        if (p->pointing_at_block)
            ui_render_block_wireframe(p);
        ui_render_crosshair();
        player_render_item(p);
    }
}

// First pass is applying depth of field effect
void render_first_pass(float dt)
{
    shader_use(shader_deferred1);

    shader_set_texture_2d(shader_deferred1, "texture_color", g_window->fb->gbuf_tex_color, 0);
    shader_set_texture_2d(shader_deferred1, "texture_depth", g_window->fb->gbuf_tex_depth, 1);

    if (!DOF_ENABLED)
    {
        shader_set_int1(shader_deferred1, "u_dof_enabled", 0);
    }
    else
    {
        float curr_depth = DOF_SMOOTH ? get_current_dof_depth(dt) : 0.0f;

        shader_set_int1(shader_deferred1, "u_dof_enabled", 1);
        shader_set_int1(shader_deferred1, "u_dof_smooth", DOF_SMOOTH);
        shader_set_float1(shader_deferred1, "u_max_blur", DOF_MAX_BLUR);
        shader_set_float1(shader_deferred1, "u_aperture", DOF_APERTURE);
        shader_set_float1(shader_deferred1, "u_aspect_ratio", 
                          (float)g_window->width / g_window->height);
        shader_set_float1(shader_deferred1, "u_depth", curr_depth);
    }

    framebuffer_use_texture(TEX_PASS_1);
    glBindVertexArray(g_window->fb->quad_vao);
    glDepthFunc(GL_ALWAYS);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Second pass is applying motion blur and color correction
void render_second_pass(Player* p, float dt)
{
    shader_use(shader_deferred2);

    shader_set_texture_2d(shader_deferred2, "texture_color", g_window->fb->gbuf_tex_color_pass_1, 0);
    shader_set_texture_2d(shader_deferred2, "texture_ui",    g_window->fb->gbuf_tex_color_ui, 1);
    shader_set_texture_2d(shader_deferred2, "texture_depth", g_window->fb->gbuf_tex_depth, 2);

    if (!MOTION_BLUR_ENABLED)
    {
        shader_set_int1(shader_deferred2, "u_motion_blur_enabled", 0);
    }
    else
    {
        shader_set_int1(shader_deferred2, "u_motion_blur_enabled", 1);
        
        mat4 matrix;
        glm_mat4_inv(p->cam->proj_matrix, matrix);
        shader_set_mat4(shader_deferred2, "u_projection_inv_matrix", matrix);

        glm_mat4_inv(p->cam->view_matrix, matrix);
        shader_set_mat4(shader_deferred2, "u_view_inv_matrix", matrix);

        glm_mat4_copy(p->cam->prev_view_matrix, matrix);
        shader_set_mat4(shader_deferred2, "u_prev_view_matrix", matrix);

        glm_mat4_copy(p->cam->proj_matrix, matrix);
        shader_set_mat4(shader_deferred2, "u_projection_matrix", matrix);

        shader_set_float3(shader_deferred2, "u_cam_pos", p->cam->pos);
        shader_set_float3(shader_deferred2, "u_prev_cam_pos", p->cam->prev_pos);

        shader_set_float1(shader_deferred2, "u_strength", MOTION_BLUR_STRENGTH);
        shader_set_int1(shader_deferred2, "u_samples", MOTION_BLUR_SAMPLES);
        shader_set_float1(shader_deferred2, "u_dt", dt);
    }

    shader_set_float1(shader_deferred2, "u_gamma", GAMMA);
    shader_set_float1(shader_deferred2, "u_saturation", SATURATION);

    framebuffer_use(g_window->fb, FBTYPE_DEFAULT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(g_window->fb->quad_vao);
    glDepthFunc(GL_ALWAYS);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render(Player* p, float dt)
{
    render_game(p);

    render_first_pass(dt);
    render_second_pass(p, dt);
}

int main()
{    
    config_load();
    window_init();
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "GLAD failed to init\n");
        glfwDestroyWindow(g_window->glfw);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

#ifdef DEBUG
    // Set up debug context if it's available
    GLint context_flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
    if (context_flags & GLFW_OPENGL_DEBUG_CONTEXT)
    {
        printf("Using debug OpenGL context\n");
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB((GLDEBUGPROCARB)opengl_debug_callback, NULL);
    }
#endif

    const GLubyte* version = glGetString(GL_VERSION);
    fprintf(stdout, "\nUsing OpenGL %s\n", version);

    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    fprintf(stdout, "Renderer: %s (%s)\n\n", vendor, renderer);

    // Start at the beginning of day
    glfwSetTime(DAY_LENGTH / 2.0);

    window_init_fb();
    db_init();
    shaders_load();
    textures_load();
    map_init();
    ui_init((float)WINDOW_WIDTH / WINDOW_HEIGHT);

    Player* player = player_create();

    // GameObjectRefs will be available in glfw callback
    // functions (using glfwGetWindowUserPointer)
    GameObjectRefs* objects = malloc(sizeof(GameObjectRefs));
    objects->player = player;
    glfwSetWindowUserPointer(g_window->glfw, objects);

    while (!glfwWindowShouldClose(g_window->glfw))
    {
        print_fps();

        float dt = get_dt();

        update(player, dt);
        render(player, dt);

        glfwSwapBuffers(g_window->glfw);
        glfwPollEvents();
    }

    map_exit();
    player_exit(player);
    db_close();

    window_destroy();

    return 0;
}