__kernel void matmul_row_major_nn_static_1(__global float const * const restrict a_buf, __global float const * const restrict b_buf, __global float * const restrict res_g_c, __global float * const restrict int_res_c) {
  const size_t i_wg_l_1 = get_group_id(2);
  const size_t i_wi_l_1 = get_local_id(2);
  const size_t i_wg_l_2 = get_group_id(0);
  const size_t i_wi_l_2 = get_local_id(0);
  const size_t i_wg_r_1 = get_group_id(1);
  const size_t i_wi_r_1 = get_local_id(1);
  size_t l_cb_offset_l_1;
  size_t l_cb_offset_l_2;
  size_t l_cb_offset_r_1;
  size_t p_cb_offset_l_1;
  size_t p_cb_offset_l_2;
  size_t p_cb_offset_r_1;
  l_cb_offset_l_1 = i_wg_l_1 * 2;
  barrier(CLK_LOCAL_MEM_FENCE);
  for (size_t l_step_l_1 = 0; l_step_l_1 < ((32 / (1 * 2)) / (16 / 2)); ++l_step_l_1) {
    barrier(CLK_LOCAL_MEM_FENCE);
    l_cb_offset_l_2 = i_wg_l_2 * 2;
    for (size_t l_step_l_2 = 0; l_step_l_2 < ((32 / (2 * 2)) / (8 / 2)); ++l_step_l_2) {
      barrier(CLK_LOCAL_MEM_FENCE);
      l_cb_offset_r_1 = i_wg_r_1 * 1;
      size_t l_step_r_1 = 0;
      p_cb_offset_l_1 = i_wi_l_1 * 1;
      for (size_t p_step_l_1 = 0; p_step_l_1 < ((16 / 2) / (2)); ++p_step_l_1) {
        p_cb_offset_l_2 = i_wi_l_2 * 1;
        for (size_t p_step_l_2 = 0; p_step_l_2 < ((8 / 2) / (2)); ++p_step_l_2) {
          p_cb_offset_r_1 = i_wi_r_1 * 1;
          size_t p_step_r_1 = 0;
#pragma unroll
          for (size_t p_iteration_l_1 = 0; p_iteration_l_1 < (2); ++p_iteration_l_1) {
#pragma unroll
            for (size_t p_iteration_l_2 = 0; p_iteration_l_2 < (2); ++p_iteration_l_2) {
              size_t p_iteration_r_1 = 0;
              int_res_c[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (1 * 2) + i_wi_l_1)) * (((32 - 1)) - (0) + 1) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2))] = a_buf[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (1 * 2) + i_wi_l_1)) * (32) + ((l_cb_offset_r_1 + (((p_cb_offset_r_1 + (((p_iteration_r_1)) / 1) * 1 + 0)) / 1) * (1 * 1) + i_wi_r_1))] * b_buf[((l_cb_offset_r_1 + (((p_cb_offset_r_1 + (((p_iteration_r_1)) / 1) * 1 + 0)) / 1) * (1 * 1) + i_wi_r_1)) * (32) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2))];
#pragma unroll
              for (p_iteration_r_1 = 1; p_iteration_r_1 < (4); ++p_iteration_r_1) {
                int_res_c[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (1 * 2) + i_wi_l_1)) * (((32 - 1)) - (0) + 1) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2))] += a_buf[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (1 * 2) + i_wi_l_1)) * (32) + ((l_cb_offset_r_1 + (((p_cb_offset_r_1 + (((p_iteration_r_1)) / 1) * 1 + 0)) / 1) * (1 * 1) + i_wi_r_1))] * b_buf[((l_cb_offset_r_1 + (((p_cb_offset_r_1 + (((p_iteration_r_1)) / 1) * 1 + 0)) / 1) * (1 * 1) + i_wi_r_1)) * (32) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2))];
              }
            }
          }
          p_cb_offset_r_1 += 1 * (4);
          for (p_step_r_1 = 1; p_step_r_1 < ((8 / 1) / (4)); ++p_step_r_1) {
#pragma unroll
            for (size_t p_iteration_l_1 = 0; p_iteration_l_1 < (2); ++p_iteration_l_1) {
              for (size_t p_iteration_l_2 = 0; p_iteration_l_2 < (2); ++p_iteration_l_2) {
                for (size_t p_iteration_r_1 = 0; p_iteration_r_1 < (4); ++p_iteration_r_1) {
                  int_res_c[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (1 * 2) + i_wi_l_1)) * (((32 - 1)) - (0) + 1) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2))] += a_buf[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (1 * 2) + i_wi_l_1)) * (32) + ((l_cb_offset_r_1 + (((p_cb_offset_r_1 + (((p_iteration_r_1)) / 1) * 1 + 0)) / 1) * (1 * 1) + i_wi_r_1))] * b_buf[((l_cb_offset_r_1 + (((p_cb_offset_r_1 + (((p_iteration_r_1)) / 1) * 1 + 0)) / 1) * (1 * 1) + i_wi_r_1)) * (32) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2))];
                }
              }
            }
            p_cb_offset_r_1 += 1 * (4);
          }
          p_cb_offset_l_2 += 2 * (2);
        }
        p_cb_offset_l_1 += 2 * (2);
      }
      l_cb_offset_r_1 += (1 * 1) * (8 / 1);
      for (l_step_r_1 = 1; l_step_r_1 < ((32 / (1 * 1)) / (8 / 1)); ++l_step_r_1) {
        p_cb_offset_l_1 = i_wi_l_1 * 1;
        for (size_t p_step_l_1 = 0; p_step_l_1 < ((16 / 2) / (2)); ++p_step_l_1) {
          p_cb_offset_l_2 = i_wi_l_2 * 1;
          for (size_t p_step_l_2 = 0; p_step_l_2 < ((8 / 2) / (2)); ++p_step_l_2) {
            p_cb_offset_r_1 = i_wi_r_1 * 1;
            size_t p_step_r_1 = 0;
#pragma unroll
            for (size_t p_iteration_l_1 = 0; p_iteration_l_1 < (2); ++p_iteration_l_1) {
              for (size_t p_iteration_l_2 = 0; p_iteration_l_2 < (2); ++p_iteration_l_2) {
                for (size_t p_iteration_r_1 = 0; p_iteration_r_1 < (4); ++p_iteration_r_1) {
                  int_res_c[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (1 * 2) + i_wi_l_1)) * (((32 - 1)) - (0) + 1) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2))] += a_buf[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (1 * 2) + i_wi_l_1)) * (32) + ((l_cb_offset_r_1 + (((p_cb_offset_r_1 + (((p_iteration_r_1)) / 1) * 1 + 0)) / 1) * (1 * 1) + i_wi_r_1))] * b_buf[((l_cb_offset_r_1 + (((p_cb_offset_r_1 + (((p_iteration_r_1)) / 1) * 1 + 0)) / 1) * (1 * 1) + i_wi_r_1)) * (32) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2))];
                }
              }
            }
            p_cb_offset_r_1 += 1 * (4);
            for (p_step_r_1 = 1; p_step_r_1 < ((8 / 1) / (4)); ++p_step_r_1) {
#pragma unroll
              for (size_t p_iteration_l_1 = 0; p_iteration_l_1 < (2); ++p_iteration_l_1) {
                for (size_t p_iteration_l_2 = 0; p_iteration_l_2 < (2); ++p_iteration_l_2) {
                  for (size_t p_iteration_r_1 = 0; p_iteration_r_1 < (4); ++p_iteration_r_1) {
                    int_res_c[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (1 * 2) + i_wi_l_1)) * (((32 - 1)) - (0) + 1) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2))] += a_buf[((l_cb_offset_l_1 + (((p_cb_offset_l_1 + (((p_iteration_l_1)) / 1) * 2 + 0)) / 2) * (1 * 2) + i_wi_l_1)) * (32) + ((l_cb_offset_r_1 + (((p_cb_offset_r_1 + (((p_iteration_r_1)) / 1) * 1 + 0)) / 1) * (1 * 1) + i_wi_r_1))] * b_buf[((l_cb_offset_r_1 + (((p_cb_offset_r_1 + (((p_iteration_r_1)) / 1) * 1 + 0)) / 1) * (1 * 1) + i_wi_r_1)) * (32) + ((l_cb_offset_l_2 + (((p_cb_offset_l_2 + (((p_iteration_l_2)) / 1) * 2 + 0)) / 2) * (2 * 2) + i_wi_l_2))];
                  }
                }
              }
              p_cb_offset_r_1 += 1 * (4);
            }
            p_cb_offset_l_2 += 2 * (2);
          }
          p_cb_offset_l_1 += 2 * (2);
        }
        l_cb_offset_r_1 += (1 * 1) * (8 / 1);
      }
      l_cb_offset_l_2 += (2 * 2) * (8 / 2);
    }
    l_cb_offset_l_1 += (1 * 2) * (16 / 2);
  }
}
