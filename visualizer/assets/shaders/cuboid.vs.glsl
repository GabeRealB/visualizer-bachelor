#version 410 core

@program mat4x4 1 view_matrix
@program mat4x4 1 projection_matrix

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_texture_coordinates;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in uint in_status_flags;
layout(location = 4) in mat4 in_model_matrix;

flat out int side;
flat out uint status_flags;
out vec2 texture_coordinates;
out vec4 vs_position;
out vec3 vs_normal;

void main () {
    mat4 view_projection_matrix = projection_matrix * view_matrix;

    side = int(in_texture_coordinates.w);
    status_flags = in_status_flags;
    texture_coordinates = in_texture_coordinates.xy;

    vs_position = view_matrix * in_model_matrix * in_position;
    vs_normal = transpose(inverse(mat3(view_projection_matrix) * mat3(in_model_matrix))) * in_normal;

    gl_Position = projection_matrix * vs_position;
}