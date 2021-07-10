#version 330 core

in vec2 v_texcoord;
in float v_ao;
flat in uint v_tile;
in float v_fog_amount;

out vec4 out_color;

uniform sampler2DArray texture_sampler;
uniform float block_light;
uniform vec3 fog_color;

void main()
{    
    vec4 color = texture(texture_sampler, vec3(v_texcoord, v_tile));
    if (color.a < 0.5)
        discard;
        
    color.rgb -= 0.35 * v_ao;
    color.rgb *= block_light;
    color.rgb = mix(color.rgb, fog_color, v_fog_amount);

    out_color = color;
}
