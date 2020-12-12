#version 330 core

flat in uint v_tile;
in vec2 v_texcoord;
out vec4 out_color;

uniform sampler2DArray texture_sampler;

void main()
{
    out_color = texture(texture_sampler, vec3(v_texcoord, v_tile));
    if (out_color.a < 0.5)
        discard;
}
