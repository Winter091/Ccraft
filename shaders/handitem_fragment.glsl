#version 330 core

in vec2 v_texcoord;
flat in uint v_tile;

out vec4 out_color;

uniform sampler2DArray texture_sampler;
uniform float block_light;

void main()
{    
    vec4 color = texture(texture_sampler, vec3(v_texcoord, v_tile));
    if (color.a < 0.5)
        discard;
    
    out_color = vec4(color.rgb * block_light, color.a);
}
