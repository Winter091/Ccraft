#ifndef UI_H_
#define UI_H_

#include "glad/glad.h"
#include "camera.h"
#include "player.h"

typedef struct
{
    GLuint VAO_crosshair;
    GLuint VBO_crosshair;

    GLuint VAO_block_wireframe;
    GLuint VBO_block_wireframe;

    GLuint VAO_cube;
    GLuint VBO_cube;

    GLuint line_shader;
    GLuint cube_shader;

    GLuint texture_blocks;
}
UI;

UI* ui_create(float aspect_ratio);
void ui_update_aspect_ratio(UI* ui, float new_ratio);
void ui_render_crosshair(UI* ui);
void ui_render_block_wireframe(UI* ui, Player* p);
void ui_render_item(UI* ui, Player* p);

#endif