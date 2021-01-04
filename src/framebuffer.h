#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include "glad/glad.h"

extern GLuint FBO_screen;
extern GLuint VAO_screen;

extern GLuint FBO_game;
extern GLuint FBO_game_texture_color;
extern GLuint FBO_game_texture_color_ui;
extern GLuint FBO_game_texture_depth;

void framebuffer_create(int window_w, int window_h);
void framebuffer_bind(GLuint framebuffer);

#endif