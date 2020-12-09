#include "utils.h"

GLuint opengl_create_vao()
{
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    return VAO;
}

GLuint opengl_create_vbo(float* vertices, size_t buf_size, size_t vert_count)
{
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, buf_size, vertices, GL_STATIC_DRAW);
    return VBO;
}