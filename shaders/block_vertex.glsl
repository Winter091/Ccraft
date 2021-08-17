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
out vec4 v_frag_pos_near_light_space;
out vec4 v_frag_pos_far_light_space;
uniform mat4 u_near_light_matrix;
uniform mat4 u_far_light_matrix;
// =================================

const vec3 normals[7] = {
    { -1.0,  0.0,  0.0 }, // 0 left
    {  1.0,  0.0,  0.0 }, // 1 right
    {  0.0,  1.0,  0.0 }, // 2 top
    {  0.0, -1.0,  0.0 }, // 3 bottom
    {  0.0,  0.0, -1.0 }, // 4 back
    {  0.0,  0.0,  1.0 }, // 5 front
    {  1.0,  1.0,  1.0 }  // 6 undefined
};

void main()
{
    gl_Position = mvp_matrix * vec4(a_pos, 1.0);
    v_pos = a_pos;
    v_ao = a_ao;
    v_tile = a_tile;
    v_texcoord = a_texcoord;

    float dist_to_cam = distance(cam_pos.xz, a_pos.xz);
    v_fog_amount = pow(clamp(dist_to_cam / fog_dist, 0.0, 1.0), 4.0);

    v_frag_pos_near_light_space = u_near_light_matrix * vec4(a_pos + u_light_dir * 0.04, 1.0);
    v_frag_pos_far_light_space = u_far_light_matrix * vec4(a_pos, 1.0);

    v_normal = normals[a_normal];
}
