#ifndef UTILS_H_
#define UTILS_H_

#include "glad/glad.h"

GLuint opengl_create_vao();
GLuint opengl_create_vbo(float* vertices, size_t buf_size);

#endif