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

void render(GLFWwindow* window, GameObjects* game)
{
    //glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    map_render_sky(game->player->cam);
    map_render_sun_moon(game->player->cam);
    map_render_chunks(game->player->cam);
    
    if (game->player->pointing_at_block)
        ui_render_block_wireframe(game->ui, game->player);
    ui_render_crosshair(game->ui);
    player_render_item(game->player);

    //glfwSwapBuffers(window);
    //glfwPollEvents();
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

    // Start at the beginning of day
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

    GLuint FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    GLuint texture_color;
    glGenTextures(1, &texture_color);
    glBindTexture(GL_TEXTURE_2D, texture_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color, 0);

    GLuint texture_depth;
    glGenTextures(1, &texture_depth);
    glBindTexture(GL_TEXTURE_2D, texture_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1280, 720, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture_depth, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Framebuffer is incomplete!\n");
        return 1;
    }

    float vertices[] = {  
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 
        1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,   0.0f, 1.0f, 
 
        -1.0f, 1.0f, 0.0f,   0.0f, 1.0f, 
        1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,    1.0f, 1.0f
    };

    GLuint VAO_screen = opengl_create_vao();
    GLuint VBO_screen = opengl_create_vbo(vertices, sizeof(vertices));
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    opengl_vbo_layout(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));

    while (!glfwWindowShouldClose(window))
    {
        print_fps();

        update(window, game);

        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render(window, game);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader_screen);
        glBindVertexArray(VAO_screen);
        texture_bind(texture_color, 0);
        shader_set_int1(shader_screen, "texture_sampler_color", 0);
        texture_bind(texture_depth, 1);
        shader_set_int1(shader_screen, "texture_sampler_depth", 1);

        int key_j = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
        int key_u = glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS;
        int key_k = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;
        int key_i = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;

        static float max_blur = 0.0101f;
        static float aperture = 0.1005f;
        float diff = 0.0003f;

        if (key_j) max_blur -= diff;
        if (key_u) max_blur += diff;

        if (key_k) aperture -= diff;
        if (key_i) aperture += diff;

        //printf("%.6f %.6f\n", max_blur, aperture);

        shader_set_float1(shader_screen, "maxBlur", max_blur);
        shader_set_float1(shader_screen, "aperture", aperture);

        glDepthFunc(GL_ALWAYS);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDepthFunc(GL_LESS);

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