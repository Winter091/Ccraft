#version 330 core

layout (location = 0) in vec3 a_pos;

uniform mat4 mvp_matrix;

void main()
{
    gl_Position = mvp_matrix * vec4(a_pos, 1.0);
}
