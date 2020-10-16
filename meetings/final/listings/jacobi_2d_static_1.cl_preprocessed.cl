//# 1 "/tmp/tmp.bmIaX6fxKf.c"
//# 1 "<built-in>"
//# 1 "<command-line>"
//# 31 "<command-line>"
//# 1 "/usr/include/stdc-predef.h" 1 3 4
//# 32 "<command-line>" 2
//# 1 "/tmp/tmp.bmIaX6fxKf.c"
//# 1318 "/tmp/tmp.bmIaX6fxKf.c"
__kernel void jacobi_2d_static_1(__global float const * const restrict input_buf, __global float * const restrict res_g_output, __global float * const restrict int_res_output) {
  const size_t i_wg_l_1 = get_group_id(1);
  const size_t i_wi_l_1 = get_local_id(1);
  const size_t i_wg_l_2 = get_group_id(0);
  const size_t i_wi_l_2 = get_local_id(0);
  size_t l_cb_offset_l_1;
  size_t l_cb_offset_l_2;
  size_t p_cb_offset_l_1;
  size_t p_cb_offset_l_2;
//# 1372 "/tmp/tmp.bmIaX6fxKf.c"
  l_cb_offset_l_1 = i_wg_l_1 * 2;
//# 1487 "/tmp/tmp.bmIaX6fxKf.c"
  barrier(CLK_LOCAL_MEM_FENCE);
//# 1999 "/tmp/tmp.bmIaX6fxKf.c"
  for (size_t l_step_l_1 = 0; l_step_l_1 < ((32 / (2 * 2)) / (8 / 2)); ++l_step_l_1) {
//# 2100 "/tmp/tmp.bmIaX6fxKf.c"
    barrier(CLK_LOCAL_MEM_FENCE);
//# 2547 "/tmp/tmp.bmIaX6fxKf.c"
    l_cb_offset_l_2 = i_wg_l_2 * 2;
    for (size_t l_step_l_2 = 0; l_step_l_2 < ((32 / (2 * 2)) / (16 / 2)); ++l_step_l_2) {
//# 3023 "/tmp/tmp.bmIaX6fxKf.c"
      p_cb_offset_l_1 = i_wi_l_1 * 1;
      for (size_t p_step_l_1 = 0; p_step_l_1 < ((8 / 2) / (2)); ++p_step_l_1) {
//# 3316 "/tmp/tmp.bmIaX6fxKf.c"
        p_cb_offset_l_2 = i_wi_l_2 * 1;
        for (size_t p_step_l_2 = 0; p_step_l_2 < ((16 / 2) / (2)); ++p_step_l_2) {
//# 3602 "/tmp/tmp.bmIaX6fxKf.c"
#pragma unroll
          for (size_t p_iteration_l_1 = 0; p_iteration_l_1 < (2); ++p_iteration_l_1) {
            for (size_t p_iteration_l_2 = 0; p_iteration_l_2 < (2); ++p_iteration_l_2) {
//# 3614 "/tmp/tmp.bmIaX6fxKf.c"
              int_res_output[(1 + (l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_1)) * (32 + 2) + (1 + (l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2))] = 0.2f * (input_buf[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_1) + 1) * (32 + 2) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2) + 1)] + input_buf[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_1) + 1) * (32 + 2) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2) + 0)] + input_buf[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_1) + 1) * (32 + 2) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2) + 2)] + input_buf[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_1) + 2) * (32 + 2) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2) + 1)] + input_buf[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_1) + 0) * (32 + 2) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2) + 1)]);
//# 3624 "/tmp/tmp.bmIaX6fxKf.c"
            }
          }
//# 3638 "/tmp/tmp.bmIaX6fxKf.c"
          p_cb_offset_l_2 += 2 * (2);
        }
//# 3950 "/tmp/tmp.bmIaX6fxKf.c"
        p_cb_offset_l_1 += 2 * (2);
      }
//# 5082 "/tmp/tmp.bmIaX6fxKf.c"
      l_cb_offset_l_2 += (2 * 2) * (16 / 2);
    }
//# 9552 "/tmp/tmp.bmIaX6fxKf.c"
    l_cb_offset_l_1 += (2 * 2) * (8 / 2);
  }
//# 22197 "/tmp/tmp.bmIaX6fxKf.c"
}
