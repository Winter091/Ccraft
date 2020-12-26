#ifndef UTILS_H_
#define UTILS_H_

#include "glad/glad.h"

GLuint opengl_create_vao();
GLuint opengl_create_vbo(void* vertices, size_t buf_size);

GLuint opengl_create_vbo_skybox();

#endif