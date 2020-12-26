#version 330 core

in vec3 v_texcoord;

out vec4 out_color;

uniform samplerCube texture_sampler;

void main()
{    
    out_color = texture(texture_sampler, v_texcoord);
}
