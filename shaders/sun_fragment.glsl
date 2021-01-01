#version 330 core

in vec2 v_texcoord;

out vec4 out_color;

uniform sampler2D texture_sampler;

void main()
{    
    out_color = texture(texture_sampler, v_texcoord);
    //float lum = 0.2126 * out_color.r + 0.7152 * out_color.g + 0.0722 * out_color.b;
    //out_color.a = lum;
}
