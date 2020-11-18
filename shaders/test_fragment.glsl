#version 330 core

in vec3 v_col;
out vec4 out_color;

void main()
{
    out_color = vec4(v_col.xyz, 1.0f);
}
