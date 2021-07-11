#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "glad/glad.h"

// Global variables for other files to access
extern GLuint texture_blocks;
extern GLuint texture_skybox_day;
extern GLuint texture_skybox_evening;
extern GLuint texture_skybox_night;
extern GLuint texture_sun;
extern GLuint texture_moon;

void textures_init();

GLuint framebuffer_color_texture_create(int width, int height);

GLuint framebuffer_depth_texture_create(int width, int height);

void texture_2d_bind(GLuint texture, int slot);

void texture_array_bind(GLuint texture, int slot);

void texture_skybox_bind(GLuint texture, int slot);

void textures_free();

#endif