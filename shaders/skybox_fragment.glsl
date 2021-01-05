#version 330 core

in vec3 v_texcoord;

out vec4 out_color;

uniform samplerCube texture_day_sampler;
uniform samplerCube texture_evening_sampler;
uniform samplerCube texture_night_sampler;

uniform float time;
uniform float day_to_evn_start;
uniform float evn_to_night_start;
uniform float night_start;
uniform float night_to_day_start;

#define PI 3.1415926535
#define COS60 0.5
#define SIN60 0.86602540378

void main()
{    
    float sint = sin(2 * PI * time);
    float cost = cos(2 * PI * time);
    
    // rotate only night texture;
    // This is 2 rotation matrices combined to one, it rotates
    // stars 60 degrees on x axis and (360 * time) degrees on y axis
    vec3 texcoord_night;
    texcoord_night.x =  v_texcoord.x * cost + v_texcoord.y * sint * SIN60 + v_texcoord.z * sint * COS60;
    texcoord_night.y =                        v_texcoord.y * COS60        - v_texcoord.z * SIN60;
    texcoord_night.z = -v_texcoord.x * sint + v_texcoord.y * cost * SIN60 + v_texcoord.z * cost * COS60;
    
    vec4 day = texture(texture_day_sampler, v_texcoord);
    vec4 evening = texture(texture_evening_sampler, v_texcoord);
    vec4 night = texture(texture_night_sampler, texcoord_night);

    // blend different skyboxes according to current time
    if (time < evn_to_night_start)
        out_color = mix(day, evening, smoothstep(day_to_evn_start, evn_to_night_start, time));
    else if (time < night_to_day_start)
        out_color = mix(evening, night, smoothstep(evn_to_night_start, night_start, time));
    else
        out_color = mix(night, day, smoothstep(night_to_day_start, 1.0, time));
}
