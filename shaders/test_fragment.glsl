#version 330 core

in vec2 v_texcoord;
out vec4 out_color;

uniform sampler2D tex_sampler;

void main()
{
    out_color = texture(tex_sampler, v_texcoord);
}
