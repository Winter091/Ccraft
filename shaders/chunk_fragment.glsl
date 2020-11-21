#version 330 core

in flat uint v_tile;

out vec4 out_color;

void main()
{
    if (v_tile == 1)
        out_color = vec4(1.0, 0.0, 0.0, 1.0);
    else if (v_tile == 2)
        out_color = vec4(0.0, 1.0, 0.0, 1.0);
    else if (v_tile == 3)
        out_color = vec4(0.0, 0.0, 1.0, 1.0);
    else
        out_color = vec4(v_tile, 0.0, v_tile, 1.0);
}
