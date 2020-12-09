#ifndef UI_H_
#define UI_H_

#include "glad/glad.h"

typedef struct
{
    GLuint VAO_crosshair;
    GLuint VBO_crosshair;
    GLuint line_shader;
}
UI;

UI* ui_create(float aspect_ratio);
void ui_update_aspect_ratio(UI* ui, float new_ratio);
void ui_render_crosshair(UI* ui);

#endif