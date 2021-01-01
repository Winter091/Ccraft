#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "cglm/cglm.h"

#include "config.h"
#include "shader.h"
#include "camera.h"
#include "window.h"
#include "map.h"
#include "texture.h"
#include "db.h"
#include "player.h"

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

static float get_dt()
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

void render(GLFWwindow* window, GameObjects* game)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    map_render_sky(game->player->cam);
    map_render_sun_moon(game->player->cam);
    map_render_chunks(game->player->cam);
    
    if (game->player->pointing_at_block)
        ui_render_block_wireframe(game->ui, game->player);
    ui_render_crosshair(game->ui);
    ui_render_hand_or_item(game->ui, game->player);

    glfwSwapBuffers(window);
    glfwPollEvents();
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

    const GLubyte* version = glGetString(GL_VERSION);
    fprintf(stdout, "Using OpenGL %s\n", version);

#if USE_DATABASE
    db_init();
#endif

    // Start at the begginning of day
    glfwSetTime(DAY_LENGTH / 2.0);

    shader_load();
    texture_load();
    map_init();

    GameObjects* game = malloc(sizeof(GameObjects));
    game->player = player_create(
        (vec3){ 0.0f, 96.0f * BLOCK_SIZE, 0.0f },
        (vec3){ 0.0f, 0.0f, 1.0f }
    );
    game->ui = ui_create((float)WINDOW_WIDTH / WINDOW_HEIGHT);

    // GameObj will be available in glfw callback
    // functions (with glfwGetWindowUserPointer)
    glfwSetWindowUserPointer(window, game);

    while (!glfwWindowShouldClose(window))
    {
        print_fps();

        update(window, game);
        render(window, game);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

#if USE_DATABASE
    db_close();
#endif

    return 0;
}