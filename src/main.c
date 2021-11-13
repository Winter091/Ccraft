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
static void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
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

static void update(Player* p, float dt)
{
    player_update(p, dt);
    map_update(p->cam);
}

// Global for this file, ideally should be inside 
// renderer file or be a part of renderer structure
static mat4 near_shadowmap_mat;
static mat4 far_shadowmap_mat;

static float near_shadowmap_size;
static float far_shadowmap_size;

typedef enum
{
    NEARPLANE_DEFAULT,
    NEARPLANE_EXTENDED
}
NearPlaneType;

static void gen_shadowmap_mat(mat4 res, Camera* cam, 
                              float size_blocks, NearPlaneType near_choice)
{
    vec3 light_dir;
    map_get_light_dir(light_dir);

    vec3 light_pos;
    glm_vec3_copy(cam->pos, light_pos);

    // Minigate shadow edge flickering
    float const discrete_step = BLOCK_SIZE / 2.0f;
    for (int i = 0; i < 3; i++)
        light_pos[i] = roundf(light_pos[i] / discrete_step) * discrete_step;

    mat4 light_view_mat;
    glm_look(light_pos, light_dir, cam->up, light_view_mat);

    float ortho_right  =  size_blocks / 2.0f * BLOCK_SIZE;
    float ortho_left   = -ortho_right;
    float ortho_top    =  size_blocks / 2.0f * BLOCK_SIZE;
    float ortho_bottom = -ortho_top;
    float ortho_far    =  size_blocks / 2.0f * BLOCK_SIZE;
    float ortho_near;

    switch (near_choice)
    {
        case NEARPLANE_DEFAULT:    
            ortho_near = -ortho_far; 
            break;
        case NEARPLANE_EXTENDED: 
            ortho_near = -(CHUNK_RENDER_RADIUS + 3) * CHUNK_WIDTH * BLOCK_SIZE;
            break;
    }

    mat4 light_proj_mat;
    glm_ortho(ortho_left, ortho_right, ortho_bottom, ortho_top, 
       ortho_near, ortho_far, light_proj_mat);

    glm_mat4_mul(light_proj_mat, light_view_mat, res);
}

static void gen_shadowmap_planes(vec4 res[6], Camera* cam, float size_blocks)
{
    mat4 light_mat;
    gen_shadowmap_mat(light_mat, cam, size_blocks, NEARPLANE_EXTENDED);
    glm_frustum_planes(light_mat, res);
}

static void render_shadowmap(mat4 light_mat, vec4 frustum_planes[6], int shadowmap_tex_width, 
                             FbType fb_type, float polygon_offset)
{
    shader_use(shader_shadow);
    shader_set_mat4(shader_shadow, "mvp_matrix", light_mat);
    shader_set_texture_array(shader_shadow, "u_blocks_texture", texture_blocks, 0);

    framebuffer_use(g_window->fb, fb_type);

    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, shadowmap_tex_width, shadowmap_tex_width);
    glPolygonOffset(polygon_offset, polygon_offset);

    map_render_chunks_raw(frustum_planes);
}

static void render_all_shadowmaps(Player* p)
{
    near_shadowmap_size = 50.0f;
    far_shadowmap_size  = (CHUNK_RENDER_RADIUS + 3) * CHUNK_WIDTH;
    
    gen_shadowmap_mat(near_shadowmap_mat, p->cam, near_shadowmap_size, NEARPLANE_DEFAULT);
    gen_shadowmap_mat(far_shadowmap_mat,  p->cam, far_shadowmap_size,  NEARPLANE_DEFAULT);

    vec4 near_planes[6], far_planes[6];
    gen_shadowmap_planes(near_planes, p->cam, near_shadowmap_size);
    gen_shadowmap_planes(far_planes,  p->cam, far_shadowmap_size);
    
    glEnable(GL_DEPTH_CLAMP);
    glEnable(GL_POLYGON_OFFSET_FILL);

    render_shadowmap(near_shadowmap_mat, near_planes, g_window->fb->near_shadowmap_w, FBTYPE_SHADOW_NEAR, 4.0f);
    render_shadowmap(far_shadowmap_mat,  far_planes,  g_window->fb->far_shadowmap_w,  FBTYPE_SHADOW_FAR,  8.0f);

    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_DEPTH_CLAMP);
}

static void render_game(Player* p)
{
    glViewport(0, 0, g_window->width, g_window->height);
    
    framebuffer_use(g_window->fb, FBTYPE_TEXTURE);
    glClear(GL_DEPTH_BUFFER_BIT);

    framebuffer_use_texture(TEX_COLOR);
    {
        map_render_sky(p->cam);
        map_render_sun_moon(p->cam);
        map_render_chunks(p->cam, near_shadowmap_mat, far_shadowmap_mat);
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
static void render_first_pass(float dt)
{
    shader_use(shader_deferred1);

    shader_set_texture_2d(shader_deferred1, "texture_color",
                          g_window->fb->gbuf_tex_color, 0);
    shader_set_texture_2d(shader_deferred1, "texture_depth",
                          g_window->fb->gbuf_tex_depth, 1);

    shader_set_int1(shader_deferred1, "u_dof_enabled", DOF_ENABLED);
    if (DOF_ENABLED)
    {
        shader_set_int1(shader_deferred1, "u_dof_smooth", DOF_SMOOTH);
        shader_set_float1(shader_deferred1, "u_max_blur", DOF_MAX_BLUR);
        shader_set_float1(shader_deferred1, "u_aperture", DOF_APERTURE);
        shader_set_float1(shader_deferred1, "u_aspect_ratio", 
                          (float)g_window->width / g_window->height);

        float curr_depth = DOF_SMOOTH ? get_current_dof_depth(dt) : 0.0f;
        shader_set_float1(shader_deferred1, "u_depth", curr_depth);
    }

    framebuffer_use_texture(TEX_PASS_1);
    glBindVertexArray(g_window->fb->quad_vao);
    glDepthFunc(GL_ALWAYS);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Apply motion blur, gamma correction, saturation and render to screen
static void render_second_pass(Player* p, float dt)
{
    shader_use(shader_deferred2);

    shader_set_texture_2d(shader_deferred2, "texture_color",
                          g_window->fb->gbuf_tex_color_pass_1, 0);
    shader_set_texture_2d(shader_deferred2, "texture_ui",
                          g_window->fb->gbuf_tex_color_ui, 1);
    shader_set_texture_2d(shader_deferred2, "texture_depth",
                          g_window->fb->gbuf_tex_depth, 2);

    shader_set_int1(shader_deferred2, "u_motion_blur_enabled", MOTION_BLUR_ENABLED);
    if (MOTION_BLUR_ENABLED)
    {
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

    // =============== Debug picture in picture shadowmaps ==================
    
    shader_use(shader_pip);
    shader_set_texture_2d(shader_pip, "u_texture", g_window->fb->gbuf_shadow_near_map, 0);
    int w = 325;
    int h = 325;
    glViewport(10, g_window->height - h - 10, w, h);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader_set_texture_2d(shader_pip, "u_texture", g_window->fb->gbuf_shadow_far_map, 0);
    glViewport(g_window->width - w - 10, g_window->height - h - 10, w, h);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
}

static void render(Player* p, float dt)
{ 
    render_all_shadowmaps(p);
    render_game(p);
    render_first_pass(dt);
    render_second_pass(p, dt);
}

int main(int argc, const char** argv)
{
    srand(time(0));

    const char* config_path;
    if (argc >= 2)
        config_path = argv[1];
    else
    {
        config_path = "config.ini";
        fprintf(stdout, "No arg was provided. Using default config path: %s\n", config_path);
    }
    config_load(config_path);
    
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
        window_update_title_fps();

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

    return 0;
}