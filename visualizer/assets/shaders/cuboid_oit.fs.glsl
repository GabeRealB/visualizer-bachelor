#version 410 core

@program float 1 far_plane
@material float 10 heatmap_color_start
@material uint 1 max_access_count
@material uint 1 heatmap_color_count
@material vec4 1 active_fill_color
@material vec4 1 out_of_bounds_fill_color
@material vec4 1 inactive_fill_color
@material vec4 10 heatmap_fill_colors
@material sampler2D 1 grid_texture_front
@material sampler2D 1 grid_texture_side
@material sampler2D 1 grid_texture_top

uniform float far_plane;
uniform float heatmap_color_start[10];
uniform uint max_access_count;
uniform uint heatmap_color_count;

uniform vec4 active_fill_color;
uniform vec4 out_of_bounds_fill_color;
uniform vec4 inactive_fill_color;
uniform vec4 heatmap_fill_colors[10];

uniform sampler2D grid_texture_front;
uniform sampler2D grid_texture_side;
uniform sampler2D grid_texture_top;

flat in int side;
flat in uint status_flags;
in vec2 texture_coordinates;
in vec4 vs_position;
in vec3 vs_normal;

layout(location = 0) out vec4 accum;
layout(location = 1) out float revealage;
layout(location = 2) out vec3 modulate;

const uint ACTIVE_FLAG = 1u << 31;
const uint OUT_OF_BOUNDS_FLAG = 1u << 30;
const uint HEATMAP_FLAG = 1u << 29;

const uint HEATMAP_COUNTER_BITS = 16;
const uint HEATMAP_COUNTER_MASK = (1u << HEATMAP_COUNTER_BITS) - 1;

float linearstep(float a, float b, float x) {
    return max(min((x - a) / (b - a), 1.0f), 0.0f);
}

vec4 fetch_heatmap_color() {
    uint heatmap_counter = (status_flags & HEATMAP_COUNTER_MASK);
    float mix_factor = max(min(float(heatmap_counter) / float(max_access_count), 1.0f), 0.0f);
    uint color_idx = 0;

    if (heatmap_color_count == 0) {
        return vec4(0.0f);
    }

    for (uint i = 0; i < heatmap_color_count; i++) {
        if (mix_factor == heatmap_color_start[i]) {
            return heatmap_fill_colors[i];
        } else if (mix_factor > heatmap_color_start[i]) {
            color_idx = i;
        }
    }

    if (color_idx + 1 == heatmap_color_count) {
        return heatmap_fill_colors[color_idx];
    } else {
        // Mix colors
        vec4 color_start = heatmap_fill_colors[color_idx];
        vec4 color_end = heatmap_fill_colors[color_idx + 1];
        float blend_factor = linearstep(heatmap_color_start[color_idx], heatmap_color_start[color_idx + 1], mix_factor);
        return mix(color_start, color_end, blend_factor);
    }
}

vec4 fetch_sample_color() {
    bool active_bit = (status_flags & ACTIVE_FLAG) != 0;
    bool out_of_bounds_bit = (status_flags & OUT_OF_BOUNDS_FLAG) != 0;
    bool heatmap_bit = (status_flags & HEATMAP_FLAG) != 0;

    if (active_bit) {
        if (out_of_bounds_bit) {
            // Draw out of bounds.
            return out_of_bounds_fill_color;
        } else if (heatmap_bit) {
            // Draw heatmap with active tones.
            return mix(active_fill_color, fetch_heatmap_color(), 0.5f);
        } else {
            // Draw normally.
            return active_fill_color;
        }
    } else if (heatmap_bit) {
        // Draw heatmap.
        return fetch_heatmap_color();
    } else {
        // Draw inactive
        return inactive_fill_color;
    }
}

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

    if (texture_color == vec4(0.0f, 0.0f, 0.0f, 1.0f)) {
        discard;
    }

    vec4 diffuse_color = fetch_sample_color();
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