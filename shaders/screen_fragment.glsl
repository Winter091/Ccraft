#version 330 core

in vec2 v_texcoord;

layout (location = 0) out vec4 out_color;

uniform sampler2D texture_sampler_color;
uniform sampler2D texture_sampler_depth;

const float near = 0.05f;
const float far  = 32 * 1.1f * 32 * 0.5f;
void main()
{
    out_color = texture(texture_sampler_color, v_texcoord);

    float depth = texture(texture_sampler_depth, v_texcoord).r;
    float ndc = depth * 2.0 - 1.0; 
    float linearDepth = (2.0 * near * far) / (far + near - ndc * (far - near));	
    vec4 out_depth = vec4(vec3(linearDepth / far), 1.0);

    out_color = mix(out_color, out_depth, 0.0);
} 
