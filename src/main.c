#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "cglm/cglm.h"

#include "config.h"
#include "shader.h"
#include "camera.h"
#include "window.h"

int main()
{    
    GLFWwindow* window = window_create();
    
    if (!gladLoadGL())
    {
        fprintf(stderr, "GLAD failed to init\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    const GLubyte* version = glGetString(GL_VERSION);
    fprintf(stdout, "Using OpenGL %s\n", version);

    GameObjects* game_obj = malloc(sizeof(GameObjects));
    game_obj->cam = camera_create((vec3){ 0.0f, 0.0f, -1.5f });

    // GameObj will be available in glfw callback
    // functions (with glfwGetWindowUserPointer)
    glfwSetWindowUserPointer(window, game_obj);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
         0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW
    );
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, 6 * sizeof(float), NULL);
    glVertexAttribPointer(1, 3, GL_FLOAT, 0, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    GLuint shader = create_shader_program(
        "shaders/test_vertex.glsl",
        "shaders/test_fragment.glsl"
    );

    double last_time = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double curr_time = glfwGetTime();
        double dt = curr_time - last_time;
        last_time = curr_time;
        
        glClearColor(0.26f, 0.32f, 0.32f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        camera_update(game_obj->cam, window, dt);

        glBindVertexArray(VAO);
        glUseProgram(shader);
        shader_set_mat4(shader, "mvp_matrix", game_obj->cam->vp_matrix);
        glDrawArrays(GL_TRIANGLES, 0, 3);
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}