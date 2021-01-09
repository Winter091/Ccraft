#version 330 core

// Motion blur and color correction shader

in vec2 v_texcoord;

out vec4 out_color;

uniform sampler2D texture_color;
uniform sampler2D texture_ui;
uniform sampler2D texture_depth;

uniform int  u_motion_blur_enabled;
uniform mat4 u_projection_inv_matrix;
uniform mat4 u_view_inv_matrix;
uniform mat4 u_prev_view_matrix;
uniform mat4 u_projection_matrix;
uniform vec3 u_cam_pos;
uniform vec3 u_prev_cam_pos;
uniform float u_strength;
uniform int u_samples;
uniform float u_dt;

uniform float u_gamma;
uniform float u_saturation;

vec3 motion_blur(vec3 rgb)
{
	float depth = texture(texture_depth, v_texcoord).r;
    vec4 curr_pos = vec4(v_texcoord, depth, 1.0) * 2.0 - 1.0;
		
	vec4 frag_pos = u_projection_inv_matrix * curr_pos;
	frag_pos = u_view_inv_matrix * frag_pos;
	frag_pos /= frag_pos.w;
	frag_pos.xyz += u_cam_pos;
	
	vec4 frag_prev_pos = frag_pos;
	frag_prev_pos.xyz -= u_prev_cam_pos;
	frag_prev_pos = u_prev_view_matrix * frag_prev_pos;
	frag_prev_pos = u_projection_matrix * frag_prev_pos;
	frag_prev_pos /= frag_prev_pos.w;

	vec2 velocity = (curr_pos - frag_prev_pos).xy * u_strength / u_dt;
	vec2 coord = v_texcoord.xy + velocity;

    vec3 color = rgb;
	for (int i = 1; i < u_samples; i++, coord += velocity) 
	{
		color += texture(texture_color, coord).xyz;
	}

	return color / u_samples;
}

vec3 gamma_correction(vec3 rgb)
{
    return pow(out_color.rgb, vec3(1.0 / u_gamma));
}

vec3 saturation(vec3 rgb)
{
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec3 intensity = vec3(dot(rgb, W));
    return mix(intensity, rgb, u_saturation);
}

void main()
{   
	// Handle UI elements
    vec4 ui_color = texture(texture_ui, v_texcoord);
    if (ui_color.a > 0.5)
    {
        out_color = ui_color;
        out_color.rgb = gamma_correction(out_color.rgb);
        out_color.rgb = saturation(out_color.rgb);
        return;
    }
	
	out_color = texture(texture_color, v_texcoord);
	out_color.a = 1.0;
	
	if (u_motion_blur_enabled != 0)
		out_color.rgb = motion_blur(out_color.rgb);

	out_color.rgb = gamma_correction(out_color.rgb);
    out_color.rgb = saturation(out_color.rgb);
}
