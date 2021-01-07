#version 330 core

in vec2 v_texcoord;
in float v_ao;
flat in uint v_tile;
in float v_fog_amount;
in float v_blend_amount;

layout (location = 0) out vec4 out_color;
layout (location = 2) out vec4 out_color_ui;

uniform sampler2DArray texture_sampler;
uniform float block_light;
uniform vec3 fog_color;

uniform int write_to_ui_texture;

void main()
{    
    vec4 color = texture(texture_sampler, vec3(v_texcoord, v_tile));
    if (color.a < 0.3)
        discard;
        
    color.xyz -= 0.35 * v_ao;

    color = mix(color, vec4(fog_color * block_light, 1.0), v_fog_amount);
    color.xyz *= block_light;

    color.a -= v_blend_amount;

    if (write_to_ui_texture == 0)
        out_color = color;
    else
        out_color_ui = color;
}
