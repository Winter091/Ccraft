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

void main()
{    
    vec4 day = texture(texture_day_sampler, v_texcoord);
    vec4 evening = texture(texture_evening_sampler, v_texcoord);
    vec4 night = texture(texture_night_sampler, v_texcoord);

    // blend different skyboxes according to current time
    if (time < evn_to_night_start)
        out_color = mix(day, evening, smoothstep(day_to_evn_start, evn_to_night_start, time));
    else if (time < night_to_day_start)
        out_color = mix(evening, night, smoothstep(evn_to_night_start, night_start, time));
    else
        out_color = mix(night, day, smoothstep(night_to_day_start, 1.0, time));
}
