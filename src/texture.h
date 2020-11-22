#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "glad/glad.h"

GLuint texture_create(const char* path);
void texture_bind(GLuint texture, int slot);

#endif