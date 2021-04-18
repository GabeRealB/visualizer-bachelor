#version 430 core

@program float 1 far_plane
@program uint 1 projection_mode
@program uimageBuffer 1 img_a_buffer
@program uimage2DMS 1 img_list_head
@material float 10 heatmap_color_start
@material uint 1 max_access_count
@material uint 1 heatmap_color_count
@material float 1 line_width
@material vec3 1 cuboid_size
@material vec4 1 active_fill_color
@material vec4 1 inactive_fill_color
@material vec4 1 oob_active_color
@material vec4 1 oob_inactive_color
@material vec4 10 heatmap_fill_colors

layout (early_fragment_tests) in;

uniform float far_plane;
uniform uint projection_mode;

uniform float heatmap_color_start[10];
uniform uint max_access_count;
uniform uint heatmap_color_count;

uniform vec4 active_fill_color;
uniform vec4 inactive_fill_color;
uniform vec4 oob_active_color;
uniform vec4 oob_inactive_color;
uniform vec4 heatmap_fill_colors[10];

uniform float line_width;
uniform vec3 cuboid_size;

flat in int side;
flat in uint status_flags;
in vec2 texture_coordinates;
in vec4 vs_position;
in vec3 vs_normal;

layout(offset=0, binding=0) uniform atomic_uint counter;
layout(rgba32ui) uniform coherent uimageBuffer img_a_buffer;
layout(r32ui) uniform coherent uimage2DMS img_list_head;

layout(location = 0) out vec4 accum;
layout(location = 1) out float revealage;
layout(location = 2) out vec3 modulate;

const uint PERSPECTIVE_PROJECTION = 0u;
const uint ORTHOGRAPHIC_PROJECTION = 1u;

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
            return oob_active_color;
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
        if (out_of_bounds_bit) {
            // Draw out of bounds.
            return oob_inactive_color;
        } else {
            return inactive_fill_color;
        }
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
        discard;
    } else {
        vec4 diffuse_color = fetch_sample_color();
        vec3 n = normalize(vs_normal);
        vec3 p = normalize(-vs_position).xyz;
        vec4 color = compute_blinn_phong(n, p, diffuse_color);

        if (color.a == 0.0f) {
            discard;
        }

        uint idx = atomicCounterIncrement(counter) + 1u;
        if (idx < imageSize(img_a_buffer)) {
            ivec2 c = ivec2(gl_FragCoord.xy);
            uvec2 fragment = uvec2(packUnorm4x8(color), floatBitsToUint(gl_FragCoord.z));
            uint prev = imageAtomicExchange(img_list_head, c, gl_SampleID, idx);
            imageStore(img_a_buffer, int(idx), uvec4(fragment, 0, prev));
        }

        color.r *= color.a;
        color.g *= color.a;
        color.b *= color.a;

        write_pixel(color, vec3(0), 0);
    }
}