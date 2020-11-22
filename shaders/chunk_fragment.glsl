#version 330 core

flat in uint v_tile;
in vec2 v_texcoord;
out vec4 out_color;

uniform sampler2D texture_sampler;

void main()
{
    // hardcoded values: probably it's ok,
    // texture's resolution won't be changed
    float block_width = 16.0 / 256.0;
    uint col = v_tile % 16u;
    uint row = v_tile / 16u;

    // map coordinates to texture atlas
    vec2 tex_coord = (v_texcoord + vec2(col, row)) * block_width;

    out_color = texture(texture_sampler, tex_coord);
}
