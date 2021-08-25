#version 330 core

in vec3 v_pos;
in vec2 v_texcoord;
in float v_ao;
flat in uint v_tile;
in float v_fog_amount;
in vec3 v_normal;

out vec4 out_color;

uniform sampler2DArray texture_sampler;
uniform float block_light;
uniform vec3 fog_color;

// =================================
in vec4 v_near_shadowmap_coord;
in vec4 v_far_shadowmap_coord;
uniform sampler2DShadow u_near_shadowmap;
uniform sampler2DShadow u_far_shadowmap;
uniform float u_near_plane;
uniform float u_far_plane;
uniform vec3  u_light_dir;
uniform float u_near_shadow_dist;
uniform float u_shadow_blend_dist;
uniform vec3 u_player_pos;
uniform mat4 u_near_light_matrix;
uniform mat4 u_far_light_matrix;
uniform float u_shadow_multiplier;
// =================================

vec2 poisson_disk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float calculate_shadow(vec4 shadowmap_coord, sampler2DShadow shadowmap, float disk_dividor)
{
    vec3 proj_coords = shadowmap_coord.xyz / shadowmap_coord.w;
    proj_coords = (proj_coords + 1.0) / 2.0;

    if (proj_coords.z > 1.0)
        return 0.0;

    float shadow_factor = 0.0;
    for (int i = 0; i < 9; i++) 
    {
        int index = int(16.0 * random(vec3(v_tile, v_normal.y, v_ao), i)) % 16;
        shadow_factor += texture(shadowmap, vec3(proj_coords.xy + poisson_disk[index] / disk_dividor, proj_coords.z));
    }
    shadow_factor /= 9.0;

    return 1.0 - shadow_factor;
}

void main()
{    
    vec4 color = texture(texture_sampler, vec3(v_texcoord, v_tile));
    if (color.a < 0.5)
        discard;

    float shadow_factor = 0.0;

    // Normal check
    if (length(v_normal) < 1.2 && dot(u_light_dir, v_normal) >= 0) 
    {
        shadow_factor = 1.0;
    }
    else
    {
        float frag_dist = distance(u_player_pos, v_pos);

        if (frag_dist < u_near_shadow_dist - u_shadow_blend_dist)
        {
            shadow_factor = calculate_shadow(v_near_shadowmap_coord, u_near_shadowmap, 3000.0);
        }
        else if (frag_dist > u_near_shadow_dist + u_shadow_blend_dist)
        {
            shadow_factor = calculate_shadow(v_far_shadowmap_coord, u_far_shadowmap, 1500.0);
        }
        else
        {
            float shadow_near = calculate_shadow(v_near_shadowmap_coord, u_near_shadowmap, 3000.0);
            float shadow_far  = calculate_shadow(v_far_shadowmap_coord,  u_far_shadowmap,  1500.0);

            float mix_factor = (frag_dist - (u_near_shadow_dist - u_shadow_blend_dist)) / (2 * u_shadow_blend_dist);
            shadow_factor = mix(shadow_near, shadow_far, mix_factor);
        }
    }

    // Smooth shadowing on small angles
    float cos_angle = max(0.0, dot(-u_light_dir, v_normal));
    float start = 0.2;
    shadow_factor += (1.0 - smoothstep(0.0, start, cos_angle));

    shadow_factor *= u_shadow_multiplier;
    shadow_factor = clamp(shadow_factor, 0.0, 1.0);

    color.rgb *= (1.0 - shadow_factor / 2.0);
    color.a += shadow_factor / 3.0;

    color.rgb -= 0.35 * v_ao;
    color.rgb *= block_light;

    color.rgb = mix(color.rgb, fog_color, v_fog_amount);

    out_color = color;
}
