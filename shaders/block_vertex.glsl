#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_texcoord;
layout (location = 3) in float a_ao;
layout (location = 2) in uint a_tile;

out vec2 v_texcoord;
out float v_ao;
flat out uint v_tile;
out float v_fog_amount;
out vec3 sky_color;

uniform mat4 mvp_matrix;
uniform vec3 cam_pos;
uniform float fog_dist;

void main()
{
    gl_Position = mvp_matrix * vec4(a_pos, 1.0);
    v_ao = a_ao;
    v_tile = a_tile;
    v_texcoord = a_texcoord;

    float dist_to_cam = distance(cam_pos.xz, a_pos.xz);
    v_fog_amount = pow(clamp(dist_to_cam / fog_dist, 0.0, 1.0), 2.0);
}
