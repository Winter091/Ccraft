#version 330 core

in vec2 v_texcoord;

out vec4 out_color;

uniform sampler2DArray texture_sampler;
uniform int block_id;

void main()
{    
    out_color = texture(texture_sampler, vec3(v_texcoord, block_id));
    if (out_color.a < 0.5)
        discard;
        
    out_color.xyz *= 0.85;
}
