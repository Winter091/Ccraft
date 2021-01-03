#version 330 core

in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

uniform sampler2D texture_sampler_color;
uniform sampler2D texture_sampler_depth;

uniform float maxBlur;
uniform float aperture;

const float aspect = 1280.0 / 720.0;
const float near = 0.5 * 0.1;
const float far = 16 * 1.1 * 0.5 * 32;

void main()
{
    vec4 color = texture(texture_sampler_color, v_texcoord);

    vec2 aspectcorrect = vec2( 1.0, aspect );

    float depth = texture(texture_sampler_depth, v_texcoord).r;
    depth *= (16 * 1.1 * 16) / far;
    
    // get focus from center of screen;
    float center_depth = texture(texture_sampler_depth, vec2(0.5, 0.5)).r;
    center_depth *= (16 * 1.1 * 16) / far;

    float factor = depth - center_depth;

    vec2 dofblur = vec2 ( clamp( factor * aperture, -maxBlur, maxBlur ) );
    vec2 dofblur9 = dofblur * 0.9;
    vec2 dofblur7 = dofblur * 0.7;
    vec2 dofblur4 = dofblur * 0.4;

    vec4 col = vec4( 0.0 );

    col += texture( texture_sampler_color, v_texcoord );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.0, 0.4 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.15, 0.37 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.29, 0.29 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.37, 0.15 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.40, 0.0 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.37, -0.15 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.29, -0.29 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.15, -0.37 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.0, -0.4 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.15, 0.37 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.29, 0.29 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.37, 0.15 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.4, 0.0 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.37, -0.15 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.29, -0.29 ) * aspectcorrect ) * dofblur );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.15, -0.37 ) * aspectcorrect ) * dofblur );

    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.15, 0.37 ) * aspectcorrect ) * dofblur9 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.37, 0.15 ) * aspectcorrect ) * dofblur9 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.37, -0.15 ) * aspectcorrect ) * dofblur9 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.15, -0.37 ) * aspectcorrect ) * dofblur9 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.15, 0.37 ) * aspectcorrect ) * dofblur9 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.37, 0.15 ) * aspectcorrect ) * dofblur9 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.37, -0.15 ) * aspectcorrect ) * dofblur9 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.15, -0.37 ) * aspectcorrect ) * dofblur9 );

    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.29, 0.29 ) * aspectcorrect ) * dofblur7 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.40, 0.0 ) * aspectcorrect ) * dofblur7 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.29, -0.29 ) * aspectcorrect ) * dofblur7 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.0, -0.4 ) * aspectcorrect ) * dofblur7 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.29, 0.29 ) * aspectcorrect ) * dofblur7 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.4, 0.0 ) * aspectcorrect ) * dofblur7 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.29, -0.29 ) * aspectcorrect ) * dofblur7 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.0, 0.4 ) * aspectcorrect ) * dofblur7 );

    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.29, 0.29 ) * aspectcorrect ) * dofblur4 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.4, 0.0 ) * aspectcorrect ) * dofblur4 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.29, -0.29 ) * aspectcorrect ) * dofblur4 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.0, -0.4 ) * aspectcorrect ) * dofblur4 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.29, 0.29 ) * aspectcorrect ) * dofblur4 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.4, 0.0 ) * aspectcorrect ) * dofblur4 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( -0.29, -0.29 ) * aspectcorrect ) * dofblur4 );
    col += texture( texture_sampler_color, v_texcoord + ( vec2( 0.0, 0.4 ) * aspectcorrect ) * dofblur4 );

    out_color = col / 41.0;
    out_color.a = 1.0;
} 
