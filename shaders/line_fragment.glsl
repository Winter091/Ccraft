#version 330 core

layout(location = 0) out vec4 out_color;
layout(location = 2) out vec4 out_color_ui;

uniform int write_to_ui_texture;

void main()
{
    if (write_to_ui_texture == 0)
        out_color = vec4(1.0);
    else
        out_color_ui = vec4(1.0);
}
