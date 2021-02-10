#version 410 core

@material sampler2DMS 1 accum_texture
@material sampler2DMS 1 revealage_texture

/* sum(rgb * a, a) */
uniform sampler2DMS accum_texture;
/* prod(1 - a) */
uniform sampler2DMS revealage_texture;

out vec4 result;

float max4(vec4 v) {
    return max(max (max (v.x, v.y), v.z), v.w);
}

void main() {
    ivec2 c = ivec2(gl_FragCoord.xy);

    float revealage = texelFetch(revealage_texture, c, gl_SampleID).r;
    if (revealage == 1.0f) {
        // Save the blending and color texture fetch cost
        discard;
    }

    vec4 accum = texelFetch(accum_texture, c, gl_SampleID);

    // Suppress overflow
    if (isinf(max4(abs(accum)))) {
        accum.rgb = vec3(accum.a);
    }

    // dst' = (accum.rgb / accum.a) * (1 - revealage) + dst
    // [dst has already been modulated by the transmission colors and coverage and the blend mode
    // inverts revealage for us]
    result = vec4(accum.rgb / max(accum.a, 0.00001), revealage);
}