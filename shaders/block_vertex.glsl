#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_texcoord;
layout (location = 2) in float a_ao;
layout (location = 3) in uint a_tile;
layout (location = 4) in uint a_normal;

out vec3 v_pos;
out vec2 v_texcoord;
out float v_ao;
flat out uint v_tile;
out float v_fog_amount;
out vec3 v_normal;

uniform mat4 mvp_matrix;
uniform vec3 cam_pos;
uniform float fog_dist;
uniform vec3  u_light_dir;

// =================================
out vec4 v_near_shadowmap_coord;
out vec4 v_far_shadowmap_coord;
uniform mat4 u_near_shadowmap_mat;
uniform mat4 u_far_shadowmap_mat;
// =================================

const vec3 normals[7] = vec3[](
    vec3(-1.0,  0.0,  0.0), // 0 left
    vec3( 1.0,  0.0,  0.0), // 1 right
    vec3( 0.0,  1.0,  0.0), // 2 top
    vec3( 0.0, -1.0,  0.0), // 3 bottom
    vec3( 0.0,  0.0, -1.0), // 4 back
    vec3( 0.0,  0.0,  1.0), // 5 front
    vec3( 1.0,  1.0,  1.0)  // 6 undefined
);

void main()
{
    gl_Position = mvp_matrix * vec4(a_pos, 1.0);
    v_pos = a_pos;
    v_ao = a_ao;
    v_tile = a_tile;
    v_texcoord = a_texcoord;

    float dist_to_cam = distance(cam_pos.xz, a_pos.xz);
    v_fog_amount = pow(clamp(dist_to_cam / fog_dist, 0.0, 1.0), 4.0);

    v_near_shadowmap_coord = u_near_shadowmap_mat * vec4(a_pos, 1.0);
    v_far_shadowmap_coord  = u_far_shadowmap_mat  * vec4(a_pos, 1.0);

    v_normal = normals[a_normal];
}
