#version 330 core

// Depth of Field shader

in vec2 v_texcoord;

layout (location = 1) out vec4 out_color;

uniform sampler2D texture_color;
uniform sampler2D texture_depth;

uniform int u_dof_enabled;
uniform int u_dof_smooth;
uniform float u_max_blur;
uniform float u_aperture;
uniform float u_aspect_ratio;
uniform float u_depth;

float dof_get_factor()
{
    float depth = texture(texture_depth, v_texcoord).r;

    float center_depth;
    if (u_dof_smooth)
        center_depth = u_depth;
    else
        center_depth = texture(texture_depth, vec2(0.5, 0.5)).r;
    
    return center_depth - depth;
}

vec3 depth_of_field(vec3 rgb)
{
    float factor = dof_get_factor();
    
    vec2 dofblur = vec2(clamp(factor * u_aperture, -u_max_blur, u_max_blur));
    vec2 dofblur9 = dofblur * 0.9;
    vec2 dofblur7 = dofblur * 0.7;
    vec2 dofblur4 = dofblur * 0.4;

    vec2 aspectcorrect = vec2(1.0, u_aspect_ratio);

    vec4 col = vec4(rgb, 1.0);
    col += texture(texture_color, v_texcoord + (vec2(0.0, 0.4)     * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(0.15, 0.37)   * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(0.29, 0.29)   * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(-0.37, 0.15)  * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(0.40, 0.0)    * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(0.37, -0.15)  * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(0.29, -0.29)  * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(-0.15, -0.37) * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(0.0, -0.4)    * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(-0.15, 0.37)  * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(-0.29, 0.29)  * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(0.37, 0.15)   * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(-0.4, 0.0)    * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(-0.37, -0.15) * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur);
    col += texture(texture_color, v_texcoord + (vec2(0.15, -0.37)  * aspectcorrect) * dofblur);

    col += texture(texture_color, v_texcoord + (vec2(0.15, 0.37)   * aspectcorrect) * dofblur9);
    col += texture(texture_color, v_texcoord + (vec2(-0.37, 0.15)  * aspectcorrect) * dofblur9);
    col += texture(texture_color, v_texcoord + (vec2(0.37, -0.15)  * aspectcorrect) * dofblur9);
    col += texture(texture_color, v_texcoord + (vec2(-0.15, -0.37) * aspectcorrect) * dofblur9);
    col += texture(texture_color, v_texcoord + (vec2(-0.15, 0.37)  * aspectcorrect) * dofblur9);
    col += texture(texture_color, v_texcoord + (vec2(0.37, 0.15)   * aspectcorrect) * dofblur9);
    col += texture(texture_color, v_texcoord + (vec2(-0.37, -0.15) * aspectcorrect) * dofblur9);
    col += texture(texture_color, v_texcoord + (vec2(0.15, -0.37)  * aspectcorrect) * dofblur9);

    col += texture(texture_color, v_texcoord + (vec2(0.29, 0.29)   * aspectcorrect) * dofblur7);
    col += texture(texture_color, v_texcoord + (vec2(0.40, 0.0)    * aspectcorrect) * dofblur7);
    col += texture(texture_color, v_texcoord + (vec2(0.29, -0.29)  * aspectcorrect) * dofblur7);
    col += texture(texture_color, v_texcoord + (vec2(0.0, -0.4)    * aspectcorrect) * dofblur7);
    col += texture(texture_color, v_texcoord + (vec2(-0.29, 0.29)  * aspectcorrect) * dofblur7);
    col += texture(texture_color, v_texcoord + (vec2(-0.4, 0.0)    * aspectcorrect) * dofblur7);
    col += texture(texture_color, v_texcoord + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur7);
    col += texture(texture_color, v_texcoord + (vec2(0.0, 0.4)     * aspectcorrect) * dofblur7);

    col += texture(texture_color, v_texcoord + (vec2(0.29, 0.29)   * aspectcorrect) * dofblur4);
    col += texture(texture_color, v_texcoord + (vec2(0.4, 0.0)     * aspectcorrect) * dofblur4);
    col += texture(texture_color, v_texcoord + (vec2(0.29, -0.29)  * aspectcorrect) * dofblur4);
    col += texture(texture_color, v_texcoord + (vec2(0.0, -0.4)    * aspectcorrect) * dofblur4);
    col += texture(texture_color, v_texcoord + (vec2(-0.29, 0.29)  * aspectcorrect) * dofblur4);
    col += texture(texture_color, v_texcoord + (vec2(-0.4, 0.0)    * aspectcorrect) * dofblur4);
    col += texture(texture_color, v_texcoord + (vec2(-0.29, -0.29) * aspectcorrect) * dofblur4);
    col += texture(texture_color, v_texcoord + (vec2(0.0, 0.4)     * aspectcorrect) * dofblur4);

    col /= 41.0;
    return col.rgb;
}

void main()
{   
    out_color = texture(texture_color, v_texcoord);
    out_color.a = 1.0;

    if (u_dof_enabled == 1)
        out_color.rgb = depth_of_field(out_color.rgb);
} 
