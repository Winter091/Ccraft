#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_texcoord;

out vec2 v_texcoord;

void main()
{
    gl_Position = vec4(a_pos.xyz, 1.0);
    v_texcoord = a_texcoord;
}
