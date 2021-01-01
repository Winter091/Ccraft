#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "glad/glad.h"

extern GLuint texture_blocks;
extern GLuint texture_skybox_day;
extern GLuint texture_skybox_evening;
extern GLuint texture_skybox_night;
extern GLuint texture_sun;
extern GLuint texture_moon;


void texture_load();

void texture_bind(GLuint texture, int slot);
void array_texture_bind(GLuint texture, int slot);
void skybox_texture_bind(GLuint texture, int slot);

#endif