#version 410 core

@program float 1 far_plane
@material vec4 1 active_border_color
@material vec4 1 inactive_border_color
@material sampler2D 1 grid_texture_front
@material sampler2D 1 grid_texture_side
@material sampler2D 1 grid_texture_top

uniform float far_plane;

uniform vec4 active_border_color;
uniform vec4 inactive_border_color;

uniform sampler2D grid_texture_front;
uniform sampler2D grid_texture_side;
uniform sampler2D grid_texture_top;

flat in int side;
flat in uint status_flags;
in vec2 texture_coordinates;
in vec4 vs_position;
in vec3 vs_normal;

out vec4 out_color;

const uint ACTIVE_FLAG = 1u << 31;
const uint OUT_OF_BOUNDS_FLAG = 1u << 30;

vec3 compute_diffuse_reflection(vec3 diffuse, vec3 vs_normal, vec3 vs_point_to_light) {
    return diffuse * max(dot(vs_normal, vs_point_to_light), 0);
}

vec3 compute_specular_reflection(vec3 specular, int specular_exponent, vec3 vs_reflection, vec3 vs_point_to_light) {
    return specular * pow(max(dot(vs_reflection, vs_point_to_light), 0), specular_exponent);
}

vec4 compute_blinn_phong(vec3 vs_normal, vec3 vs_view, vec4 diffuse_color) {
    vec3 ambient = vec3(0.2f);

    vec3 light_direction = vec3(0.0f, 0.0f, 1.0f);
    vec3 h = normalize(-light_direction + vs_view);

    vec3 light_intensity = vec3(0.4f);
    vec3 diffuse_reflection = compute_diffuse_reflection(diffuse_color.xyz, vs_normal, -light_direction);
    vec3 specular_reflection = compute_specular_reflection(vec3(0.1), 1, h, vs_normal);

    vec3 color = ambient + light_intensity * (diffuse_reflection + specular_reflection);

    return vec4(color, diffuse_color.a);
}

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

    vec4 diffuse_color;

    if (texture_color == vec4(0.0f, 0.0f, 0.0f, 1.0f)) {
        if ((status_flags & ACTIVE_FLAG) != 0) {
            diffuse_color = active_border_color;
        } else {
            diffuse_color = inactive_border_color;
        }
    } else {
        discard;
    }

    vec3 n = normalize(vs_normal);
    vec3 p = normalize(-vs_position).xyz;
    vec4 color = compute_blinn_phong(n, p, diffuse_color);
    color.a = 1.0f;

    if (color.a == 0.0f) {
        discard;
    }

    out_color = color;
}