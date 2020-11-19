#ifndef SHADER_H_
#define SHADER_H_

#include "glad/glad.h"
#include "cglm/cglm.h"

GLuint create_shader_program(const char* vs_path, const char* fs_path);

void shader_set_mat4(GLuint shader, char* name, mat4 matrix);

#endif