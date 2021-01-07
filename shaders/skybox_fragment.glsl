#version 330 core

in vec3 v_texcoord;
in vec3 v_texcoord_night;

out vec4 out_color;

uniform samplerCube texture_day;
uniform samplerCube texture_evening;
uniform samplerCube texture_night;

uniform float time;
uniform float day_to_evn_start;
uniform float evn_to_night_start;
uniform float night_start;
uniform float night_to_day_start;

void main()
{      
    vec4 day = texture(texture_day, v_texcoord);
    vec4 evening = texture(texture_evening, v_texcoord);
    vec4 night = texture(texture_night, v_texcoord_night);

    // blend different skyboxes according to current time
    if (time < evn_to_night_start)
        out_color = mix(day, evening, smoothstep(day_to_evn_start, evn_to_night_start, time));
    else if (time < night_to_day_start)
        out_color = mix(evening, night, smoothstep(evn_to_night_start, night_start, time));
    else
        out_color = mix(night, day, smoothstep(night_to_day_start, 1.0, time));
}
