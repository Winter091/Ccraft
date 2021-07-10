#version 330 core

layout (location = 0) in vec3 a_pos;

out vec3 v_texcoord;
out vec3 v_texcoord_night;
out vec3 v_pos;

uniform mat4 mvp_matrix;
uniform float time;

#define PI 3.1415926535
#define COS60 0.5
#define SIN60 0.86602540378

void main()
{
    vec4 pos = mvp_matrix * vec4(a_pos, 1.0);
    gl_Position = pos.xyww;
    v_texcoord = a_pos;
    v_pos = a_pos;

    float sint = sin(2 * PI * time);
    float cost = cos(2 * PI * time);

    // Rotate only night texture;
    // This is 2 rotation matrices combined to one, it rotates
    // stars 60 degrees on x axis and (360 * time) degrees on y axis
    v_texcoord_night.x =  v_texcoord.x * cost + v_texcoord.y * sint * SIN60 + v_texcoord.z * sint * COS60;
    v_texcoord_night.y =                        v_texcoord.y * COS60        - v_texcoord.z * SIN60;
    v_texcoord_night.z = -v_texcoord.x * sint + v_texcoord.y * cost * SIN60 + v_texcoord.z * cost * COS60;
}
