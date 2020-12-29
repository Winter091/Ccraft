#ifndef SHADER_H_
#define SHADER_H_

#include "glad/glad.h"
#include "cglm/cglm.h"

extern GLuint shader_block;
extern GLuint shader_line;
extern GLuint shader_skybox;

void shader_load();

void shader_set_int1(GLuint shader, char* name, int value);
void shader_set_mat4(GLuint shader, char* name, mat4 matrix);

#endif