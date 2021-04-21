#version 430 core

@material uimage2DMS 1 img_list_head
@material uimageBuffer 1 img_a_buffer

/* Pixel list heads */
layout(r32ui) uniform coherent uimage2DMS img_list_head;
/* Per-pixel linked list */
layout(rgba32ui) uniform coherent uimageBuffer img_a_buffer;

out vec4 result;

const uint TRANSPARENCY_LAYERS = 8;

struct FragmentListNode {
    vec4 color;
    float depth;
    uint layer;
    uint next;
};

uint get_list_head() {
    return imageLoad(img_list_head, ivec2(gl_FragCoord.xy), gl_SampleID).r;
}

FragmentListNode get_stored_node(uint idx) {
    uvec4 buffer_node = imageLoad(img_a_buffer, int(idx));
    return FragmentListNode(unpackUnorm4x8(buffer_node.r), uintBitsToFloat(buffer_node.g), buffer_node.b, buffer_node.a);
}

void sort_fragments(inout FragmentListNode fragments[TRANSPARENCY_LAYERS], int count) {
    /* Src: https://github.com/gangliao/Order-Independent-Transparency-GPU/blob/master/source%20code/src/shader/oit.fs */
    int i, j1, j2, k;
    int a, b, c;
    int step = 1;
    FragmentListNode left_array[TRANSPARENCY_LAYERS/2]; //for merge sort

    while (step <= count) {
        i = 0;
        while (i < count - step) {
            ////////////////////////////////////////////////////////////////////////
            //merge(step, i, i + step, min(i + step + step, count));
            a = i;
            b = i + step;
            c = (i + step + step) >= count ? count : (i + step + step);

            for (k = 0; k < step; k++)
            left_array[k] = fragments[a + k];

            j1 = 0;
            j2 = 0;
            for (k = a; k < c; k++) {
                if (b + j1 >= c || (j2 < step && left_array[j2].depth > fragments[b + j1].depth))
                fragments[k] = left_array[j2++];
                else
                fragments[k] = fragments[b + j1++];
            }
            ////////////////////////////////////////////////////////////////////////
            i += 2 * step;
        }
        step *= 2;
    }
}

vec4 blend_frament(vec4 dst_color, vec4 src_color) {
    // SRC blend: GL_SRC_ALPHA
    // DST blend: GL_ONE_MINUS_SRC_ALPHA
    return (src_color * src_color.a) + (dst_color * (1.0f - src_color.a));
}

void main() {
    uint i = 0;
    uint idx = get_list_head();
    FragmentListNode fragments[TRANSPARENCY_LAYERS];

    // Extract fragments
    while (idx != 0 && i < TRANSPARENCY_LAYERS) {
        fragments[i] = get_stored_node(idx);
        idx = fragments[i].next;
        i++;
    }

    // Sort fragments
    sort_fragments(fragments, int(i));

    vec4 color = vec4(0);
    for(int j = 0; j < i; j++) {
        color = blend_frament(color, fragments[j].color);
    }

    result = color;
}