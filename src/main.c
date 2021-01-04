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

void update(GLFWwindow* window, GameObjects* game)
{
    player_update(game->player, window, get_dt());
    map_update(game->player->cam);
}

void render_game(GameObjects* game)
{
    map_render_sky(game->player->cam);
    map_render_sun_moon(game->player->cam);
    map_render_chunks(game->player->cam);
    
    if (game->player->pointing_at_block)
        ui_render_block_wireframe(game->ui, game->player);
    ui_render_crosshair(game->ui);
    player_render_item(game->player);
}

void render_game_quad(GameObjects* game)
{
    glUseProgram(shader_screen);

    shader_set_texture_2d(shader_screen, "texture_sampler_color",    FBO_game_texture_color, 0);
    shader_set_texture_2d(shader_screen, "texture_sampler_color_ui", FBO_game_texture_color_ui, 1);
    shader_set_texture_2d(shader_screen, "texture_sampler_depth",    FBO_game_texture_depth, 2);

    shader_set_float1(shader_screen, "u_max_blur", DOF_MAX_BLUR);
    shader_set_float1(shader_screen, "u_aperture", DOF_APERTURE);
    shader_set_float1(shader_screen, "u_aspect_ratio", (float)window_w / window_h);
    shader_set_float1(shader_screen, "u_gamma", GAMMA_CORRECTION);
    shader_set_float1(shader_screen, "u_saturation", SATURATION);

    glDepthFunc(GL_ALWAYS);

    glBindVertexArray(VAO_screen);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDepthFunc(GL_LESS);
}

void render(GLFWwindow* window, GameObjects* game)
{
    framebuffer_bind(FBO_game);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_game(game);

    framebuffer_bind(FBO_screen);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_game_quad(game);
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
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

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

        update(window, game);
        render(window, game);

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