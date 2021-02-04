#version 410 core

@program float 1 far_plane
@material vec4 1 active_fill_color
@material vec4 1 inactive_fill_color
@material vec4 1 active_border_color
@material vec4 1 inactive_border_color
@material sampler2D 1 grid_texture_front
@material sampler2D 1 grid_texture_side
@material sampler2D 1 grid_texture_top

uniform float far_plane;

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

    float camera_distance = (gl_FragCoord.z / gl_FragCoord.w) / far_plane;
    float border_thickness = mix(0.0f, 0.1f, camera_distance);

    vec2 tex_coords = texture_coordinates;

    if (tex_coords.x < border_thickness) {
        tex_coords.x = 0.0f;
    }
    if (tex_coords.x > 1 - border_thickness) {
        tex_coords.x = 1.0f;
    }
    if (tex_coords.y < border_thickness) {
        tex_coords.y = 0.0f;
    }
    if (tex_coords.y > 1 - border_thickness) {
        tex_coords.y = 1.0f;
    }

    if (side == 0) {
        texture_color = vec4(texture(grid_texture_front, tex_coords).xyz, 1.0f);
    } else if (side == 1) {
        texture_color = vec4(texture(grid_texture_top, tex_coords).xyz, 1.0f);
    } else if (side == 2) {
        texture_color = vec4(texture(grid_texture_side, tex_coords).xyz, 1.0f);
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