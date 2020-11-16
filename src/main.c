#include "stdio.h"
#include "stdlib.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

int main()
{
    if (!glfwInit())
    {
        printf("GLFW not init\n");
        exit(EXIT_FAILURE);
    }
    
    GLFWwindow* window = glfwCreateWindow(640, 480, "Window", NULL, NULL);
    if (!window)
    {
        printf("GLFW window not init\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL())
    {
        printf("GLAD not init\n");
        exit(EXIT_FAILURE);
    }

    int width = 1280, height = 720;

    while (!glfwWindowShouldClose(window))
    {
        glViewport(0, 0, width, height);
        glClearColor(0.5, 0.3, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}