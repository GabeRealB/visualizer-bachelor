#version 410 core

@program float 1 far_plane
@program uint 1 projection_mode
@material float 1 line_width
@material vec3 1 cuboid_size
@material vec4 1 active_border_color
@material vec4 1 inactive_border_color

uniform float far_plane;
uniform uint projection_mode;

uniform vec4 active_border_color;
uniform vec4 inactive_border_color;

uniform float line_width;
uniform vec3 cuboid_size;

flat in int side;
flat in uint status_flags;
in vec2 texture_coordinates;
in vec4 vs_position;
in vec3 vs_normal;

out vec4 out_color;

const uint PERSPECTIVE_PROJECTION = 0u;
const uint ORTHOGRAPHIC_PROJECTION = 1u;

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

bool is_border() {
    vec2 proj_size;
    if (side == 0) {
        proj_size = vec2(cuboid_size[0], cuboid_size[1]);
    } else if (side == 1) {
        proj_size = vec2(cuboid_size[0], cuboid_size[2]);
    } else if (side == 2) {
        proj_size = vec2(cuboid_size[2], cuboid_size[1]);
    }

    vec2 pos = texture_coordinates * proj_size;

    float border_magnification = 1.0f;
    if (projection_mode == PERSPECTIVE_PROJECTION) {
        float camera_distance = (gl_FragCoord.z / gl_FragCoord.w) / far_plane;
        border_magnification = mix(1.0f, 2.0f, camera_distance);
    } else if (projection_mode == ORTHOGRAPHIC_PROJECTION) {
        border_magnification = mix(1.0f, 2.0f, far_plane);
    }

    float border_width = line_width * border_magnification;

    if (pos.x <= border_width ||
        proj_size[0] - border_width <= pos.x ||
        pos.y <= border_width ||
        proj_size[1] - border_width <= pos.y) {
        return true;
    } else {
        return false;
    }
}

void main() {
    if (is_border()) {
        vec4 diffuse_color;
        if ((status_flags & ACTIVE_FLAG) != 0) {
            diffuse_color = active_border_color;
        } else {
            diffuse_color = inactive_border_color;
        }

        vec3 n = normalize(vs_normal);
        vec3 p = normalize(-vs_position).xyz;
        vec4 color = compute_blinn_phong(n, p, diffuse_color);
        color.a = 1.0f;

        if (color.a == 0.0f) {
            discard;
        }

        out_color = color;
    } else {
        discard;
    }
}