#ifndef SHADER_H_
#define SHADER_H_

#include "glad/glad.h"
#include "cglm/cglm.h"

// Global variables for other files to access
extern GLuint shader_block;
extern GLuint shader_line;
extern GLuint shader_skybox;
extern GLuint shader_sun;
extern GLuint shader_deferred1;
extern GLuint shader_deferred2;

void shaders_init();

void shader_set_int1(GLuint shader, char* name, int value);

void shader_set_float1(GLuint shader, char* name, float value);

void shader_set_float3(GLuint shader, char* name, vec3 vec);

void shader_set_mat4(GLuint shader, char* name, mat4 matrix);

void shader_set_texture_2d(GLuint shader, char* name, GLuint texture, int slot);

void shader_set_texture_array(GLuint shader, char* name, GLuint texture, int slot);

void shader_set_texture_skybox(GLuint shader, char* name, GLuint texture, int slot);

void shader_use(GLuint shader);

void shaders_free();

#endif