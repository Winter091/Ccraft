#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in uint a_tile;

out flat uint v_tile;

uniform mat4 mvp_matrix;

void main()
{
    gl_Position = mvp_matrix * vec4(a_pos, 1.0);
    v_tile = a_tile;
}
