#version 330 core

in vec2 v_texcoord;
flat in uint v_tile;

uniform sampler2DArray u_blocks_texture;

void main()
{      
    vec4 color = texture(u_blocks_texture, vec3(v_texcoord, v_tile));
    if (color.a < 0.5)
        discard;

    //gl_FragDepth = gl_FragCoord.z;
}
