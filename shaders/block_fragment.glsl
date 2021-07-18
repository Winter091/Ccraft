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
in vec4 v_frag_pos_near_light_space;
in vec4 v_frag_pos_far_light_space;
uniform sampler2D u_near_shadow_map;
uniform sampler2D u_far_shadow_map;
uniform float u_near_plane;
uniform float u_far_plane;
// =================================

float calculate_shadow(vec4 frag_pos_light_space, sampler2D shadow_map)
{
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords = (proj_coords + 1.0) / 2.0;

    if (proj_coords.z > 1.0)
        return 0.0;

    float closest_depth = texture(shadow_map, proj_coords.xy).r;
    float curr_depth = proj_coords.z;

    float bias = 0.005;
    float shadow_factor = curr_depth - bias > closest_depth ? 1.0 : 0.0;

    return shadow_factor;
}

float linearize_depth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    float res = (2.0 * u_near_plane * u_far_plane) / (u_far_plane + u_near_plane - z * (u_far_plane - u_near_plane));
    return res / u_far_plane;
}

void main()
{    
    vec4 color = texture(texture_sampler, vec3(v_texcoord, v_tile));
    if (color.a < 0.5)
        discard;
        
    float frag_depth = gl_FragCoord.z;
    frag_depth = linearize_depth(frag_depth);

    float shadow_factor = 0.0;
    if (frag_depth < 0.02)
        shadow_factor = calculate_shadow(v_frag_pos_near_light_space, u_near_shadow_map);
    else
        shadow_factor = calculate_shadow(v_frag_pos_far_light_space, u_far_shadow_map);

    color *= (1.0 - shadow_factor / 2.0);

    color.rgb -= 0.35 * v_ao;
    color.rgb *= block_light;
    color.rgb = mix(color.rgb, fog_color, v_fog_amount);


    out_color = color;
}
