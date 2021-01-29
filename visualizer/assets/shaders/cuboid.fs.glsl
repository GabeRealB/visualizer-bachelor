#version 410 core

@material vec4 1 active_fill_color
@material vec4 1 inactive_fill_color
@material vec4 1 active_border_color
@material vec4 1 inactive_border_color
@material sampler2D 1 grid_texture_front
@material sampler2D 1 grid_texture_side
@material sampler2D 1 grid_texture_top

uniform vec4 active_fill_color;
uniform vec4 inactive_fill_color;
uniform vec4 active_border_color;
uniform vec4 inactive_border_color;

uniform sampler2D grid_texture_front;
uniform sampler2D grid_texture_side;
uniform sampler2D grid_texture_top;

flat in int side;
flat in uint enabled;
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

    if (texture_color == vec4(0.0f, 0.0f, 0.0f, 1.0f)) {
        if (enabled == 1) {
            out_color = active_border_color;
        } else {
            out_color = inactive_border_color;
        }
    } else {
        if (enabled == 1) {
            out_color = active_fill_color;
        } else {
            out_color = inactive_fill_color;
        }
    }

    if (out_color.a == 0.0f) {
        discard;
    }
}