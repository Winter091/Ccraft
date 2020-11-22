#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_texcoord;
layout (location = 2) in uint a_tile;

flat out uint v_tile;
out vec2 v_texcoord;

uniform mat4 mvp_matrix;

void main()
{
    gl_Position = mvp_matrix * vec4(a_pos, 1.0);
    v_tile = a_tile;
    v_texcoord = a_texcoord;
}
