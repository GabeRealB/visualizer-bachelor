#version 410 core

@program mat4x4 1 view_projection_matrix

uniform mat4 view_projection_matrix;

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_texture_coordinates;
layout(location = 2) in uint in_enabled;
layout(location = 3) in mat4 in_model_matrix;

flat out int side;
flat out uint enabled;
out vec2 texture_coordinates;

void main () {
    side = int(in_texture_coordinates.w);
    enabled = in_enabled;
    texture_coordinates = in_texture_coordinates.xy;
    gl_Position = view_projection_matrix * in_model_matrix * in_position;
}