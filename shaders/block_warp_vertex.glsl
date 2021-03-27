#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_texcoord;
layout (location = 3) in float a_ao;
layout (location = 2) in uint a_tile;

out vec2 v_texcoord;
out float v_ao;
flat out uint v_tile;
out float v_fog_amount;

uniform mat4 mvp_matrix;
uniform vec3 cam_pos;
uniform float fog_dist;

uniform float time;

#define PI 3.1415926538

float map(float value, float min1, float max1, float min2, float max2) 
{
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main()
{
    gl_Position = vec4(a_pos, 1.0);
    v_ao = a_ao;
    v_tile = a_tile;
    v_texcoord = a_texcoord;

    float dist_to_cam = distance(cam_pos.xz, a_pos.xz);
    float dist_x = distance(cam_pos.x, a_pos.x);
    float dist_y = distance(cam_pos.y, a_pos.y);
    v_fog_amount = 0.001 * pow(clamp(dist_to_cam / fog_dist, 0.0, 1.0), 2.0);

    gl_Position.y += 25 * sin(dist_to_cam * sin(time * 0.075) * 0.5) * min(dist_to_cam / 100, 1);
    //gl_Position.y += 145 * sin(dist_to_cam * 0.025) * min(dist_to_cam / 100, 1) * sin(time * 0.7);
    float x = gl_Position.x;
    float y = gl_Position.y;
    float om = sin(dist_to_cam * sin(time * 0.1) * 0.1) * sin(time * 0.75);
    om *= 0.25;
    om *= min(dist_to_cam / 50, 1);
    gl_Position.x = x * cos(om) - y * sin(om);
    gl_Position.y = x * sin(om) + y * cos(om);

    //gl_Position.y += 0.05 * dist_to_cam * dist_to_cam;
    //if (dist_to_cam > 2)
    //    gl_Position.y += 5;

    //gl_Position.x = cam_pos.x + 10 * (sin(dist_x / 10 + 3.1415 / 2) - 1);

    // Huge cylinder!
    //float r = 10;
    //gl_Position.x = cam_pos.x + r * sin(dist_x / r) * sign(a_pos.x - cam_pos.x);
    //gl_Position.y += (r - dist_y / 2) * (sin(dist_x / r - PI / 2) + 1);
    
    gl_Position = mvp_matrix * gl_Position;
}
