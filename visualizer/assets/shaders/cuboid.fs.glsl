#version 410 core

@material vec4 1 fill_color
@material vec4 1 border_color
@material sampler2D 1 grid_texture_front
@material sampler2D 1 grid_texture_side
@material sampler2D 1 grid_texture_top

uniform vec4 fill_color;
uniform vec4 border_color;

uniform sampler2D grid_texture_front;
uniform sampler2D grid_texture_side;
uniform sampler2D grid_texture_top;

flat in int side;
in vec2 texture_coordinates;
out vec4 out_color;

void main() {
    vec4 texture_color = vec4(0.0f);

    if (side == 0) {
        texture_color = vec4(texture(grid_texture_front, texture_coordinates).xyz, 1.0f);
    } else if (side == 1) {
        texture_color = vec4(texture(grid_texture_top, texture_coordinates).xyz, 1.0f);
    } else if (side == 2) {
        texture_color = vec4(texture(grid_texture_side, texture_coordinates).xyz, 1.0f);
    }

    out_color = texture_color * border_color;

    if (texture_color == vec4(0.0f, 0.0f, 0.0f, 1.0f)) {
        //out_color = vec4(0.6f, 0.6f, 0.6f, 1.0f);
        out_color = border_color;
    } else {
        out_color = fill_color;
    }

    if (out_color.a == 0.0f) {
        discard;
    }
}