#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "cglm/cglm.h"

#include "config.h"
#include "shader.h"
#include "camera.h"

GLFWwindow* window_create()
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

    GLFWmonitor* monitor = FULLSCREEN ? glfwGetPrimaryMonitor() : NULL;

    // setup windowed full screen mode
    if (monitor)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);
    }

    // will fail if OpenGL version is not supported
    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, 
        WINDOW_TITLE, monitor, NULL
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(VSYNC);

    return window;
}

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

    const GLubyte* version = glGetString(GL_VERSION);
    fprintf(stdout, "Using OpenGL %s\n", version);

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

    Camera* cam = camera_create((vec3){0.0f, 0.0f, -1.5f});

    double last_time = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double curr_time = glfwGetTime();
        double dt = curr_time - last_time;
        last_time = curr_time;
        
        glClearColor(0.26f, 0.32f, 0.32f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        camera_update(cam, window, dt);

        glBindVertexArray(VAO);
        glUseProgram(shader);
        shader_set_mat4(shader, "mvp_matrix", cam->vp_matrix);
        glDrawArrays(GL_TRIANGLES, 0, 3);
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}