#version 330 core

in vec2 v_texcoord;
in float v_ao;
flat in uint v_tile;
in float v_fog_amount;
in float v_blend_amount;

out vec4 out_color;

uniform sampler2DArray texture_sampler;
uniform float block_light;
uniform vec3 fog_color;

void main()
{    
    out_color = texture(texture_sampler, vec3(v_texcoord, v_tile));
    if (out_color.a < 0.3)
        discard;
        
    out_color.xyz -= 0.35 * v_ao;

    out_color = mix(out_color, vec4(fog_color * block_light, 1.0), v_fog_amount);
    out_color.xyz *= block_light;

    out_color.a -= v_blend_amount;
}
