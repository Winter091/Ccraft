#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_texcoord;

out vec2 v_texcoord;

uniform mat4 mvp_matrix;

void main()
{
    vec4 pos = mvp_matrix * vec4(a_pos, 1.0);
    gl_Position = pos.xyww;
    v_texcoord = a_texcoord;
}
