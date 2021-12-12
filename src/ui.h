#ifndef UI_H_
#define UI_H_

#include <glad/glad.h>

#include <camera/camera.h>
#include <player/player.h>

void ui_init(float aspect_ratio);

void ui_update_aspect_ratio(float new_ratio);

void ui_render_crosshair();

void ui_render_block_wireframe(Player* p, Camera* cam);

void ui_free();

#endif