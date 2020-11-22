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
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const GLubyte* version = glGetString(GL_VERSION);
    fprintf(stdout, "Using OpenGL %s\n", version);

    GameObjects* game_obj = malloc(sizeof(GameObjects));
    game_obj->cam = camera_create((vec3){ 0.0f, 0.0f, 3.0f });

    // GameObj will be available in glfw callback
    // functions (with glfwGetWindowUserPointer)
    glfwSetWindowUserPointer(window, game_obj);

    /*
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,     1.0f, 0.0f,
         0.0f,  0.5f, 0.0f,     0.5f, 1.0f
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
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, 5 * sizeof(float), NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    
    GLuint shader_test = create_shader_program(
        "shaders/test_vertex.glsl",
        "shaders/test_fragment.glsl"
    );
    */

    GLuint texture_blocks = texture_create("textures/blocks.png");
    if (!texture_blocks)
    {
        fprintf(stderr, "Texture was not loaded!\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    GLuint shader_chunk = create_shader_program(
        "shaders/chunk_vertex.glsl",
        "shaders/chunk_fragment.glsl"
    );

    Map* map = map_create(); 
    for (int i = 0; i < 1; i++)
        for (int j = 0; j < 1; j++)
            map_add_chunk(map, i, j);

    double last_time = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        double curr_time = glfwGetTime();
        double dt = curr_time - last_time;
        last_time = curr_time;
        
        glClearColor(0.26f, 0.32f, 0.32f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera_update(game_obj->cam, window, dt);

        
        glUseProgram(shader_chunk);
        shader_set_mat4(shader_chunk, "mvp_matrix", game_obj->cam->vp_matrix);
        shader_set_int1(shader_chunk, "texture_sampler", 0);
        texture_bind(texture_blocks, 0);

        for (int i = 0; i < map->chunk_count; i++)
        {
            glBindVertexArray(map->chunks[i]->VAO);
            glDrawArrays(GL_TRIANGLES, 0, map->chunks[i]->vertex_count);
        }
        
        /*
        glBindVertexArray(VAO);
        glUseProgram(shader_test);

        mat4 mvp;
        mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, (vec3){0.0f, 0.0f, 0.0f});
        glm_mat4_mul(game_obj->cam->vp_matrix, model, mvp);
        shader_set_mat4(shader_test, "mvp_matrix", mvp);

        texture_bind(texture_test, 0);
        shader_set_int1(shader_test, "tex_sampler", 0);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);
        */
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}