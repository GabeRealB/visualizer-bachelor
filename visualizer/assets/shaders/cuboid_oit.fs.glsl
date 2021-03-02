#version 410 core

@program float 1 far_plane
@material vec4 1 active_fill_color
@material vec4 1 out_of_bounds_fill_color
@material vec4 1 inactive_fill_color
@material sampler2D 1 grid_texture_front
@material sampler2D 1 grid_texture_side
@material sampler2D 1 grid_texture_top

uniform float far_plane;

uniform vec4 active_fill_color;
uniform vec4 out_of_bounds_fill_color;
uniform vec4 inactive_fill_color;

uniform sampler2D grid_texture_front;
uniform sampler2D grid_texture_side;
uniform sampler2D grid_texture_top;

flat in int side;
flat in uint enabled;
in vec2 texture_coordinates;
in vec4 vs_position;
in vec3 vs_normal;

layout(location = 0) out vec4 accum;
layout(location = 1) out float revealage;
layout(location = 2) out vec3 modulate;

const uint DISABLED = 0;
const uint ENABLED = 1;
const uint OUT_OF_BOUNDS = 2;

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

/* Src: https://casual-effects.blogspot.com/2015/03/colored-blended-order-independent.html */
void write_pixel(vec4 premultiplied_reflect, vec3 transmit, float csZ) {
    /* NEW: Perform this operation before modifying the coverage to account for transmission. */
    modulate = premultiplied_reflect.a * (vec3(1.0) - transmit);

    /* Modulate the net coverage for composition by the transmission. This does not affect the color channels of the
       transparent surface because the caller's BSDF model should have already taken into account if transmission modulates
       reflection. See

       McGuire and Enderton, Colored Stochastic Shadow Maps, ACM I3D, February 2011
       http://graphics.cs.williams.edu/papers/CSSM/

       for a full explanation and derivation.*/
    premultiplied_reflect.a *= 1.0 - (transmit.r + transmit.g + transmit.b) * (1.0 / 3.0);

    // Intermediate terms to be cubed
    float tmp = (premultiplied_reflect.a * 8.0 + 0.01) * (-gl_FragCoord.z * 0.95 + 1.0);

    /* If a lot of the scene is close to the far plane, then gl_FragCoord.z does not
       provide enough discrimination. Add this term to compensate:

       tmp /= sqrt(abs(csZ)); */

    float w    = clamp(tmp * tmp * tmp * 1e3, 1e-2, 3e2);
    accum     = premultiplied_reflect * w;
    revealage = premultiplied_reflect.a;
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
        discard;
    } else {
        if (enabled == ENABLED) {
            diffuse_color = active_fill_color;
        } else if (enabled == OUT_OF_BOUNDS) {
            diffuse_color = out_of_bounds_fill_color;
        } else if (enabled == DISABLED) {
            diffuse_color = inactive_fill_color;
        }
    }

    vec3 n = normalize(vs_normal);
    vec3 p = normalize(-vs_position).xyz;
    vec4 color = compute_blinn_phong(n, p, diffuse_color);

    color.r *= color.a;
    color.g *= color.a;
    color.b *= color.a;

    if (color.a == 0.0f) {
        discard;
    }

    write_pixel(color, vec3(0), 0);
}