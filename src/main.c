#include "stdio.h"
#include "stdlib.h"

#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "cglm/cglm.h"

#include "config.h"
#include "shader.h"
#include "window.h"
#include "texture.h"
#include "db.h"

// Print OpenGL warnings and errors
void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                           GLsizei length, const GLchar* message, const void* userParam)
{
    char* _source;
    char* _type;
    char* _severity;

    switch (source)
    {
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
    }

    switch (type)
    {
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
    }

    switch (severity)
    {
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
    }

    fprintf(stderr,"%d: %s of %s severity, raised from %s: %s\n",
            id, _type, _severity, _source, message);
}

static void window_title_update_fps()
{
    static double last_time = -1.0;
    if (last_time < 0)
        last_time = glfwGetTime();

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

static double get_dt()
{
    static double last_time = -1.0;
    if (last_time < 0)
        last_time = glfwGetTime();

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
    player_update(p, dt);
    map_update(p->cam);
}

mat4 light_matrix;

void render_shadow_depth(Player* p)
{
    static float n = -50.0f;
    static float f =  50.0f;
    static float s =  10.0f;
    static float yaw = -142.0f, pitch = -38.0f;

    float const delta = 0.1f;
    float const delta2 = 0.25f;

    if (window_is_key_pressed(GLFW_KEY_R)) f += delta;
    if (window_is_key_pressed(GLFW_KEY_F)) f -= delta;
    if (window_is_key_pressed(GLFW_KEY_T)) n += delta;
    if (window_is_key_pressed(GLFW_KEY_G)) n -= delta;
    if (window_is_key_pressed(GLFW_KEY_Y)) s += delta;
    if (window_is_key_pressed(GLFW_KEY_H)) s -= delta;

    if (window_is_key_pressed(GLFW_KEY_J)) yaw += delta2;
    if (window_is_key_pressed(GLFW_KEY_L)) yaw -= delta2;
    if (window_is_key_pressed(GLFW_KEY_I)) pitch += delta2;
    if (window_is_key_pressed(GLFW_KEY_K)) pitch -= delta2;

    printf("n: %8.4f f: %8.4f s: %8.4f yaw: %8.4f pitch: %8.4f\n", n, f, s, yaw, pitch);
    
    float near_plane = n * BLOCK_SIZE;
    float far_plane  = f * BLOCK_SIZE;
    float left   = -s * BLOCK_SIZE;
    float right  = s * BLOCK_SIZE;
    float bottom = -s * BLOCK_SIZE;
    float top    = s * BLOCK_SIZE;

    // ============== Create MVP matrix ===================
    mat4 proj;
    glm_ortho(left, right, bottom, top, near_plane, far_plane, proj);

    vec3 light_pos;
    glm_vec3_copy(p->cam->pos, light_pos);

    vec3 view_dir;
    view_dir[0] = cosf(glm_rad(yaw)) * cosf(glm_rad(pitch));
    view_dir[1] = sinf(glm_rad(pitch));
    view_dir[2] = sinf(glm_rad(yaw)) * cosf(glm_rad(pitch));
    glm_vec3_normalize(view_dir);

    mat4 view;
    glm_look(light_pos, view_dir, p->cam->up, view);

    glm_mat4_mul(proj, view, light_matrix);
    
    // ============== Prepare shader ===================
    shader_use(shader_shadow);
    shader_set_mat4(shader_shadow, "mvp_matrix", light_matrix);
    shader_set_texture_array(shader_shadow, "u_blocks_texture", texture_blocks, 0);

    // ============== Prepare framebuffer ===================
    glViewport(0, 0, g_window->fb->shadow_map_w, g_window->fb->shadow_map_h);
    framebuffer_use(g_window->fb, FBTYPE_SHADOW);
    glClear(GL_DEPTH_BUFFER_BIT);

    // ============== Render terrain ===================
    map_render_chunks_raw();
}

void render_game(Player* p)
{
    glViewport(0, 0, g_window->width, g_window->height);
    
    framebuffer_use(g_window->fb, FBTYPE_TEXTURE);
    glClear(GL_DEPTH_BUFFER_BIT);

    framebuffer_use_texture(TEX_COLOR);
    {
        map_render_sky(p->cam);
        map_render_sun_moon(p->cam);
        map_render_chunks(p->cam, light_matrix);
    }

    framebuffer_use_texture(TEX_UI);
    {
        if (p->pointing_at_block)
            ui_render_block_wireframe(p);
        ui_render_crosshair();
        player_render_item(p);
    }
}

// Apply depth of field and render to texture
void render_first_pass(float dt)
{
    shader_use(shader_deferred1);

    shader_set_texture_2d(shader_deferred1, "texture_color",
                          g_window->fb->gbuf_tex_color, 0);
    shader_set_texture_2d(shader_deferred1, "texture_depth",
                          g_window->fb->gbuf_tex_depth, 1);

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

// Apply motion blur, gamma correction, saturation and render to screen
void render_second_pass(Player* p, float dt)
{
    shader_use(shader_deferred2);

    shader_set_texture_2d(shader_deferred2, "texture_color",
                          g_window->fb->gbuf_tex_color_pass_1, 0);
    shader_set_texture_2d(shader_deferred2, "texture_ui",
                          g_window->fb->gbuf_tex_color_ui, 1);
    shader_set_texture_2d(shader_deferred2, "texture_depth",
                          g_window->fb->gbuf_tex_depth, 2);

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

    // =============== Picture in picture ==================
    shader_use(shader_pip);
    shader_set_texture_2d(shader_pip, "u_texture", g_window->fb->gbuf_shadow_tex_depth, 0);
    int w = 400;
    int h = 275;
    glViewport(g_window->width - w - 10, g_window->height - h - 10, w, h);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render(Player* p, float dt)
{
    render_shadow_depth(p);
    render_game(p);
    render_first_pass(dt);
    render_second_pass(p, dt);
}

int main()
{
    srand(time(0));
    
    config_init();
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
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

#ifdef DEBUG
    // Set up OpenGL's debug context, if it's available
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
    shaders_init();
    textures_init();
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
        window_title_update_fps();

        float dt = get_dt();

        update(player, dt);
        render(player, dt);

        glfwSwapBuffers(g_window->glfw);
        glfwPollEvents();
    }

    player_destroy(player);

    ui_free();
    map_free();
    textures_free();
    shaders_free();
    db_free();

    window_free();
    config_free();

    return 0;
}