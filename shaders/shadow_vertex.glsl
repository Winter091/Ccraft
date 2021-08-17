#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_texcoord;
layout (location = 2) in float a_ao;
layout (location = 3) in uint a_tile;
layout (location = 4) in uint a_normal;

out vec2 v_texcoord;
flat out uint v_tile;

uniform mat4 mvp_matrix;

void main()
{
    gl_Position = mvp_matrix * vec4(a_pos, 1.0);
    v_texcoord = a_texcoord;
    v_tile = a_tile;
}
