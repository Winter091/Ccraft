#version 330 core

in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

uniform sampler2D texture_sampler_color;
uniform sampler2D texture_sampler_color_ui;
uniform sampler2D texture_sampler_depth;

uniform float u_max_blur;
uniform float u_aperture;
uniform float u_aspect_ratio;
uniform float u_gamma;
uniform float u_saturation;
uniform float u_depth;

vec3 depth_of_field(vec3 rgb)
{
    float depth = texture(texture_sampler_depth, v_texcoord).r;
    //float center_depth = texture(texture_sampler_depth, vec2(0.5, 0.5)).r;
    float factor = u_depth - depth;
    
    vec2 dofblur = vec2(clamp(factor * u_aperture, -u_max_blur, u_max_blur));
    vec2 dofblur9 = dofblur * 0.9;
    vec2 dofblur7 = dofblur * 0.7;
    vec2 dofblur4 = dofblur * 0.4;

    vec2 aspectcorrect = vec2(1.0, u_aspect_ratio);

    vec4 col = vec4(rgb, 1.0);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.0, 0.4)     * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.15, 0.37)   * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.29, 0.29)   * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.37, 0.15)  * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.40, 0.0)    * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.37, -0.15)  * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.29, -0.29)  * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.15, -0.37) * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.0, -0.4)    * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.15, 0.37)  * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.29, 0.29)  * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.37, 0.15)   * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.4, 0.0)    * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.37, -0.15) * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.15, -0.37)  * aspectcorrect) * dofblur);

    col += texture(texture_sampler_color, v_texcoord + (vec2(0.15, 0.37)   * aspectcorrect) * dofblur9);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.37, 0.15)  * aspectcorrect) * dofblur9);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.37, -0.15)  * aspectcorrect) * dofblur9);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.15, -0.37) * aspectcorrect) * dofblur9);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.15, 0.37)  * aspectcorrect) * dofblur9);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.37, 0.15)   * aspectcorrect) * dofblur9);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.37, -0.15) * aspectcorrect) * dofblur9);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.15, -0.37)  * aspectcorrect) * dofblur9);

    col += texture(texture_sampler_color, v_texcoord + (vec2(0.29, 0.29)   * aspectcorrect) * dofblur7);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.40, 0.0)    * aspectcorrect) * dofblur7);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.29, -0.29)  * aspectcorrect) * dofblur7);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.0, -0.4)    * aspectcorrect) * dofblur7);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.29, 0.29)  * aspectcorrect) * dofblur7);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.4, 0.0)    * aspectcorrect) * dofblur7);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur7);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.0, 0.4)     * aspectcorrect) * dofblur7);

    col += texture(texture_sampler_color, v_texcoord + (vec2(0.29, 0.29)   * aspectcorrect) * dofblur4);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.4, 0.0)     * aspectcorrect) * dofblur4);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.29, -0.29)  * aspectcorrect) * dofblur4);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.0, -0.4)    * aspectcorrect) * dofblur4);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.29, 0.29)  * aspectcorrect) * dofblur4);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.4, 0.0)    * aspectcorrect) * dofblur4);
    col += texture(texture_sampler_color, v_texcoord + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur4);
    col += texture(texture_sampler_color, v_texcoord + (vec2(0.0, 0.4)     * aspectcorrect) * dofblur4);

    col /= 41.0;
    return col.rgb;
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
    vec4 ui_color = texture(texture_sampler_color_ui, v_texcoord);
    if (ui_color != vec4(1.0, 0.0, 1.0, 1.0))
    {
        vec4 color = texture(texture_sampler_color, v_texcoord);
        out_color = vec4(1 - color.xyz, 1.0);
        return;
    }
    
    // If no UI on that pixel, render game image
    out_color = texture(texture_sampler_color, v_texcoord);
    out_color.a = 1.0;

    // Postprocessing
    out_color.rgb = depth_of_field(out_color.rgb);
    out_color.rgb = gamma_correction(out_color.rgb);
    out_color.rgb = saturation(out_color.rgb);
} 
