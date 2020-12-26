#version 330 core

layout (location = 0) in vec3 a_pos;

out vec3 v_texcoord;

uniform mat4 mvp_matrix;

void main()
{
    vec4 pos = mvp_matrix * vec4(a_pos, 1.0);
    gl_Position = pos.xyww;
    v_texcoord = a_pos;
}
