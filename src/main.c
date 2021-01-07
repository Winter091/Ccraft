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
#include "shader.h"
#include "utils.h"
#include "framebuffer.h"

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
        printf("%d\n", frames);
        frames = 0;
        last_time = curr_time;
    }
}

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
    // Very slow function, on i5-3470 and r9 280 it takes about 2ms
    
    static float curr_depth = 1.0f;

    // Read depth in the center of the screen 
    // and gradually move towards it
    float desired_depth;
    glReadPixels(window_w / 2, window_h / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &desired_depth);
    curr_depth = glm_lerp(curr_depth, desired_depth, dt * DOF_SPEED);
    
    return curr_depth;
}

void update(GLFWwindow* window, GameObjects* game, float dt)
{
    player_update(game->player, window, dt);
    map_update(game->player->cam);
}

void render_game(GameObjects* game)
{
    // Render everything to textures
    framebuffer_bind(FBO_game);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    map_render_sky(game->player->cam);
    map_render_sun_moon(game->player->cam);
    map_render_chunks(game->player->cam);
    
    if (game->player->pointing_at_block)
        ui_render_block_wireframe(game->ui, game->player);
    ui_render_crosshair(game->ui);
    player_render_item(game->player);
}

// First pass is applying depth of field effect
void render_first_pass(GameObjects* game, float dt)
{
    glUseProgram(shader_deferred1);

    shader_set_texture_2d(shader_deferred1, "texture_color", FBO_game_texture_color, 0);
    shader_set_texture_2d(shader_deferred1, "texture_depth", FBO_game_texture_depth, 1);

#if DOF_ENABLED
    float curr_depth = get_current_dof_depth(dt);

    shader_set_int1(shader_deferred1, "u_dof_enabled", 1);
    shader_set_int1(shader_deferred1, "u_dof_smooth", DOF_SMOOTH);
    shader_set_float1(shader_deferred1, "u_max_blur", DOF_MAX_BLUR);
    shader_set_float1(shader_deferred1, "u_aperture", DOF_APERTURE);
    shader_set_float1(shader_deferred1, "u_aspect_ratio", (float)window_w / window_h);
    shader_set_float1(shader_deferred1, "u_depth", curr_depth);
#else
    shader_set_int1(shader_deferred1, "u_dof_enabled", 0);
#endif

    glDepthFunc(GL_ALWAYS);
    glBindVertexArray(VAO_screen);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Second pass is applying motion blur and color correction
void render_second_pass(GameObjects* game, float dt)
{
    glUseProgram(shader_deferred2);

    // Bind second color texture in which DoF is applied
    shader_set_texture_2d(shader_deferred2, "texture_color", FBO_game_texture_color2, 0);
    shader_set_texture_2d(shader_deferred2, "texture_ui",    FBO_game_texture_color_ui, 1);
    shader_set_texture_2d(shader_deferred2, "texture_depth", FBO_game_texture_depth, 2);

#if MOTION_BLUR_ENABLED
    shader_set_int1(shader_deferred2, "u_motion_blur_enabled", 1);
    
    mat4 matrix;
    glm_mat4_inv(game->player->cam->proj_matrix, matrix);
    shader_set_mat4(shader_deferred2, "u_projection_inv_matrix", matrix);

    glm_mat4_inv(game->player->cam->view_matrix, matrix);
    shader_set_mat4(shader_deferred2, "u_view_inv_matrix", matrix);

    glm_mat4_copy(game->player->cam->prev_view_matrix, matrix);
    shader_set_mat4(shader_deferred2, "u_prev_view_matrix", matrix);

    glm_mat4_copy(game->player->cam->proj_matrix, matrix);
    shader_set_mat4(shader_deferred2, "u_projection_matrix", matrix);

    shader_set_float3(shader_deferred2, "u_cam_pos", game->player->cam->pos);
    shader_set_float3(shader_deferred2, "u_prev_cam_pos", game->player->cam->prev_pos);

    shader_set_float1(shader_deferred2, "u_strength", MOTION_BLUR_STRENGTH);
    shader_set_int1(shader_deferred2, "u_samples", MOTION_BLUR_SAMPLES);
    shader_set_float1(shader_deferred2, "u_dt", dt);
#else
    shader_set_int1(shader_deferred2, "u_motion_blur_enabled", 0);
#endif

    shader_set_float1(shader_deferred2, "u_gamma", GAMMA_CORRECTION);
    shader_set_float1(shader_deferred2, "u_saturation", SATURATION);

    // It's final pass, render to default frame buffer
    framebuffer_bind(FBO_screen);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDepthFunc(GL_ALWAYS);
    glBindVertexArray(VAO_screen);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render(GLFWwindow* window, GameObjects* game, float dt)
{
    render_game(game);

    render_first_pass(game, dt);
    render_second_pass(game, dt);
}

int main()
{    
    GLFWwindow* window = window_create();
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "GLAD failed to init\n");
        glfwDestroyWindow(window);
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

    const GLubyte* version = glGetString(GL_VERSION);
    fprintf(stdout, "Using OpenGL %s\n", version);

#if USE_DATABASE
    db_init();
#endif

    // Start at the beginning of day
    glfwSetTime(DAY_LENGTH / 2.0);

    shaders_load();
    textures_load();
    map_init();
    framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);

    GameObjects* game = malloc(sizeof(GameObjects));
    game->player = player_create(
        (vec3){ 0.0f, 96.0f * BLOCK_SIZE, 0.0f },
        (vec3){ 0.0f, 0.0f, 1.0f }
    );
    game->ui = ui_create((float)WINDOW_WIDTH / WINDOW_HEIGHT);

    // GameObjects will be available in glfw callback
    // functions (using glfwGetWindowUserPointer)
    glfwSetWindowUserPointer(window, game);

    while (!glfwWindowShouldClose(window))
    {
        print_fps();

        float dt = get_dt();

        update(window, game, dt);
        render(window, game, dt);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

#if USE_DATABASE
    db_close();
#endif

    return 0;
}