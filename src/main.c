#include "stdio.h"
#include "stdlib.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "config.h"
#include "shader.h"

GLFWwindow* init_libs_create_window()
{
    if (!glfwInit())
    {
        fprintf(stderr, "GLFW failed to init.\n");
        exit(EXIT_FAILURE);
    }

    // require to use particular version of OpenGL
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR_REQUIRED);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR_REQUIRED);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // will fail if openGL version is not supported
    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, 
        WINDOW_TITLE, NULL, NULL
    );

    if (!window)
    {
        fprintf(stderr, "GLFW window failed to init.\n");
        fprintf(stderr, 
            "Probably, the required version of OpenGL is not supported:\n"
        );
        fprintf(stderr, "OpenGL %d.%d\n", 
            OPENGL_VERSION_MAJOR_REQUIRED, OPENGL_VERSION_MINOR_REQUIRED
        );
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL())
    {
        fprintf(stderr, "GLAD failed to init\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    const GLubyte* version = glGetString(GL_VERSION);
    fprintf(stdout, "Using OpenGL %s\n", version);

    return window;
}

int main()
{
    GLFWwindow* window = init_libs_create_window();

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f, 
         0.0f,  0.5f, 0.0f
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
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    GLuint shader = create_shader_program(
        "shaders/test_vertex.glsl",
        "shaders/test_fragment.glsl"
    );

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.5, 0.3, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);
        glUseProgram(shader);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}