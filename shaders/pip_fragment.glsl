#version 330 core

in vec2 v_texcoord;

out vec4 out_color;

uniform sampler2D u_texture;

void main()
{      
    float tex_color = texture(u_texture, v_texcoord).r;
    out_color = vec4(vec3(tex_color), 1.0);
}
