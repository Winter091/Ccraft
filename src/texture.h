#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "glad/glad.h"

GLuint texture_create(const char* path);
GLuint array_texture_create(const char* path);
GLuint skybox_texture_create(const char* paths[6]);

void texture_bind(GLuint texture, int slot);
void array_texture_bind(GLuint texture, int slot);
void skybox_texture_bind(GLuint texture, int slot);

#endif