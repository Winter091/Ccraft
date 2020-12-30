#version 330 core

in vec2 v_texcoord;
in float v_ao;
flat in uint v_tile;

out vec4 out_color;

uniform sampler2DArray texture_sampler;
uniform float block_light;

void main()
{    
    out_color = texture(texture_sampler, vec3(v_texcoord, v_tile));
    if (out_color.a < 0.5)
        discard;
        
    out_color -= 0.35 * v_ao;
    out_color.xyz *= block_light;
}
