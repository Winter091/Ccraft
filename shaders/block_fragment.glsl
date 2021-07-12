#version 330 core

in vec2 v_texcoord;
in float v_ao;
flat in uint v_tile;
in float v_fog_amount;

out vec4 out_color;

uniform sampler2DArray texture_sampler;
uniform float block_light;
uniform vec3 fog_color;

// =================================
in vec4 v_frag_pos_light_space;
uniform sampler2D u_shadow_map;
// =================================

float calculate_shadow(vec4 frag_pos_light_space)
{
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords = (proj_coords + 1.0) / 2.0;

    if (proj_coords.z > 1.0)
        return 0.0;

    float closest_depth = texture(u_shadow_map, proj_coords.xy).r;
    float curr_depth = proj_coords.z;

    float bias = 0.005;
    float shadow_factor = curr_depth - bias > closest_depth ? 1.0 : 0.0;

    return shadow_factor;
}

void main()
{    
    vec4 color = texture(texture_sampler, vec3(v_texcoord, v_tile));
    if (color.a < 0.5)
        discard;
        
    color.rgb -= 0.35 * v_ao;
    color.rgb *= block_light;
    color.rgb = mix(color.rgb, fog_color, v_fog_amount);

    float shadow_factor = calculate_shadow(v_frag_pos_light_space);
    color *= (1.0 - shadow_factor / 2.0);

    out_color = color;
}
