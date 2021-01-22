#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include "glad/glad.h"

extern GLuint FBO_screen;
extern GLuint VAO_screen;

extern GLuint FBO_game;
extern GLuint FBO_game_texture_color;
extern GLuint FBO_game_texture_color_ui;
extern GLuint FBO_game_texture_color_pass_1;
extern GLuint FBO_game_texture_depth;

void framebuffer_create(int curr_window_w, int curr_window_h);

// We need to rebuild framebuffer each time
// when window size changes
void framebuffer_rebuild(int curr_window_w, int curr_window_h);

void framebuffer_bind(GLuint framebuffer);

#endif