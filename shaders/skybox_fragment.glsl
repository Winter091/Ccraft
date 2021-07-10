#version 330 core

in vec3 v_texcoord;
in vec3 v_texcoord_night;
in vec3 v_pos;

out vec4 out_color;

uniform samplerCube texture_day;
uniform samplerCube texture_evening;
uniform samplerCube texture_night;

uniform float time;
uniform float day_to_evn_start;
uniform float evn_to_night_start;
uniform float night_start;
uniform float night_to_day_start;

uniform vec3 fog_color;

void main()
{      
    vec4 day = texture(texture_day, v_texcoord);
    vec4 evening = texture(texture_evening, v_texcoord);
    vec4 night = texture(texture_night, v_texcoord_night);

    // Blend different skyboxes according to current time
    if (time < evn_to_night_start)
        out_color = mix(day, evening, smoothstep(day_to_evn_start, evn_to_night_start, time));
    else if (time < night_to_day_start)
        out_color = mix(evening, night, smoothstep(evn_to_night_start, night_start, time));
    else
        out_color = mix(night, day, smoothstep(night_to_day_start, 1.0, time));
    
    vec3 dir_point    = normalize(v_pos);
    vec3 proj_to_xz   = normalize(vec3(dir_point.x, 0.0, dir_point.z));
    float cos_horizon = dot(dir_point, proj_to_xz);

    // Make fog appear slightly above horizon and
    // everywhere below it
    float fog_amount = smoothstep(0.99, 1.0, cos_horizon);
    if (v_pos.y < 0.0)
        fog_amount = 1;

    out_color = mix(out_color, vec4(fog_color, 1.0), fog_amount);
}
