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
    camera_update(game->cam, window, get_dt());
    map_update(game->map, game->cam);
}

void render(GLFWwindow* window, GameObjects* game)
{
    glClearColor(0.34f, 0.53f, 0.76f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    map_render_chunks(game->map, game->cam);

    if (game->cam->has_active_block)
        ui_render_block_wireframe(game->ui, game->cam);

    ui_render_crosshair(game->ui);

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

    db_init();

    GameObjects* game = malloc(sizeof(GameObjects));
    game->cam = camera_create((vec3){ 0.0f, 75.0f, 0.0f });
    game->map = map_create();
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

    db_close();

    return 0;
}