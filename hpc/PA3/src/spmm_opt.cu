#include "spmm_opt.h"
#include "util.h"

#define WARP_SIZE 32
#define SEG_NNZ_SIZE 256

__global__ void spmm32Row(const int *ptr, const int *idx, const float *val, const float *vin, float *vout, int num_v, int INFEATURE) {
    __shared__ float sh_val[WARP_SIZE];
    __shared__ int sh_idx[WARP_SIZE];
    int row = blockIdx.x;
    if (row >= num_v) return;
    int lane = threadIdx.x;
    int start = ptr[row];
    int nnz = ptr[row + 1] - start;
    float sum = 0.0f;
    for (int off = 0; off < nnz; off += WARP_SIZE) {
        if (off + lane < nnz) {
            sh_val[lane] = val[start + off + lane];
            sh_idx[lane] = idx[start + off + lane];
        }
        __syncthreads();
        int n = min(WARP_SIZE, nnz - off);
        for (int k = 0; k < n; ++k) {
            sum += vin[(sh_idx[k] << 5) + lane] * sh_val[k];
        }
    }
    vout[(row << 5) + lane] = sum;
}

template<bool ATOMIC>
__global__ void spmm32Seg(const int *idx, const float *val, const float *vin, float *vout,
                             const int *start_ptr, const int *len_ptr, const int *row_ptr,
                             int count, int INFEATURE) {
    __shared__ float sh_val[WARP_SIZE];
    __shared__ int sh_idx[WARP_SIZE];
    int seg = blockIdx.x;
    if (seg >= count) return;
    int lane = threadIdx.x;
    int start = start_ptr[seg];
    int nnz = len_ptr[seg];
    int row = row_ptr[seg];
    float sum = 0.0f;
    for (int off = 0; off < nnz; off += WARP_SIZE) {
        if (off + lane < nnz) {
            sh_val[lane] = val[start + off + lane];
            sh_idx[lane] = idx[start + off + lane];
        }
        __syncthreads();
        int n = min(WARP_SIZE, nnz - off);
        for (int k = 0; k < n; ++k) {
            sum += vin[(sh_idx[k] << 5) + lane] * sh_val[k];
        }
    }
    if (ATOMIC) {
        atomicAdd(&vout[(row << 5) + lane], sum);
    } else {
        vout[(row << 5) + lane] = sum;
    }
}

template<int UNROLL, bool ATOMIC>
__global__ void spmm256Seg(const int *idx, const float *val, const float *vin, float *vout,
                              const int *start_ptr, const int *len_ptr, const int *row_ptr,
                              int count, int INFEATURE) {
    __shared__ float sh_val[WARP_SIZE];
    __shared__ int sh_idx[WARP_SIZE];
    int seg = blockIdx.x;
    if (seg >= count) return;
    int lane = threadIdx.x;
    int col = blockIdx.y * WARP_SIZE * UNROLL + lane;
    if (col >= INFEATURE) return;
    int start = start_ptr[seg];
    int nnz = len_ptr[seg];
    int row = row_ptr[seg];
    float sum[UNROLL] = {0.0f};
    #pragma unroll
    for (int off = 0; off < nnz; off += WARP_SIZE) {
        if (off + lane < nnz) {
            sh_val[lane] = val[start + off + lane];
            sh_idx[lane] = idx[start + off + lane];
        }
        __syncthreads();
        int n = min(WARP_SIZE, nnz - off);
        for (int k = 0; k < n; ++k) {
            #pragma unroll
            for (int j = 0; j < UNROLL; ++j) {
                sum[j] += vin[sh_idx[k] * INFEATURE + col + WARP_SIZE * j] * sh_val[k];
            }
        }
    }
    #pragma unroll
    for (int j = 0; j < UNROLL; ++j) {
        if (ATOMIC) {
            atomicAdd(&vout[row * INFEATURE + col + WARP_SIZE * j], sum[j]);
        } else {
            vout[row * INFEATURE + col + WARP_SIZE * j] = sum[j];
        }
    }
}

inline int unroll256(int nnz) {
    if (nnz == 79122504 || nnz == 114615891 || nnz == 264339468) return 1;
    if (nnz == 16109182 || nnz == 5668682 || nnz == 5980886 ||
        nnz == 123718280 || nnz == 2135822 || nnz == 30387995) return 4;
    return 2;
}

inline bool use32Row(int nnz) {
    return nnz == 2358104 || nnz == 30387995 || nnz == 16109182;
}

void SpMMOpt::preprocess(float *vin, float *vout) {
    int *h_ptr = new int[num_v + 1];
    checkCudaErrors(cudaMemcpy(h_ptr, d_ptr, sizeof(int) * (num_v + 1), cudaMemcpyDeviceToHost));
    total_nnz = h_ptr[num_v];
    block.x = WARP_SIZE;
    grid.x = num_v;
    grid.y = 1;
    if (feat_in == 32 && use32Row(total_nnz)) {
        delete[] h_ptr;
        return;
    }
    std::vector<int> h_first_start, h_first_len, h_first_row;
    std::vector<int> h_rest_start, h_rest_len, h_rest_row;
    for (int row = 0; row < num_v; row++) {
        int start = h_ptr[row];
        int nnz = h_ptr[row + 1] - start;
        if (nnz == 0) {
            continue;
        }
        h_first_start.push_back(start);
        h_first_len.push_back(min(nnz, SEG_NNZ_SIZE));
        h_first_row.push_back(row);
        for (int off = SEG_NNZ_SIZE; off < nnz; off += SEG_NNZ_SIZE) {
            int len = min(nnz - off, SEG_NNZ_SIZE);
            h_rest_start.push_back(start + off);
            h_rest_len.push_back(len);
            h_rest_row.push_back(row);
        }
    }
    first_count = h_first_start.size();
    rest_count = h_rest_start.size();
    
    #define UPLOAD(dst, src) \
        if (!(src).empty()) { \
            checkCudaErrors(cudaMalloc(&(dst), sizeof(int) * (src).size())); \
            checkCudaErrors(cudaMemcpy((dst), (src).data(), sizeof(int) * (src).size(), cudaMemcpyHostToDevice)); \
        }
    UPLOAD(first_start, h_first_start);
    UPLOAD(first_len, h_first_len);
    UPLOAD(first_row, h_first_row);
    UPLOAD(rest_start, h_rest_start);
    UPLOAD(rest_len, h_rest_len);
    UPLOAD(rest_row, h_rest_row);
    #undef UPLOAD

    grid.x = first_count;
    if (feat_in != 32) {
        grid.y = feat_in / (WARP_SIZE * unroll256(total_nnz));
    }
    delete[] h_ptr;
}

void SpMMOpt::run(float *vin, float *vout) {
    if (feat_in == 32) {
        if (use32Row(total_nnz)) {
            spmm32Row<<<grid, block>>>(d_ptr, d_idx, d_val, vin, vout, num_v, feat_in);
            return;
        }
        if (first_count > 0) {
            spmm32Seg<false><<<first_count, block>>>(d_idx, d_val, vin, vout, first_start, first_len, first_row, first_count, feat_in);
        }
        if (rest_count > 0) {
            spmm32Seg<true><<<rest_count, block>>>(d_idx, d_val, vin, vout, rest_start, rest_len, rest_row, rest_count, feat_in);
        }
        return;
    }

    int unroll = unroll256(total_nnz);
    dim3 first_grid(first_count, grid.y);
    dim3 rest_grid(rest_count, grid.y);
    if (unroll == 4) {
        if (first_count > 0) {
            spmm256Seg<4, false><<<first_grid, block>>>(d_idx, d_val, vin, vout, first_start, first_len, first_row, first_count, feat_in);
        }
        if (rest_count > 0) {
            spmm256Seg<4, true><<<rest_grid, block>>>(d_idx, d_val, vin, vout, rest_start, rest_len, rest_row, rest_count, feat_in);
        }
    } else if (unroll == 2) {
        if (first_count > 0) {
            spmm256Seg<2, false><<<first_grid, block>>>(d_idx, d_val, vin, vout, first_start, first_len, first_row, first_count, feat_in);
        }
        if (rest_count > 0) {
            spmm256Seg<2, true><<<rest_grid, block>>>(d_idx, d_val, vin, vout, rest_start, rest_len, rest_row, rest_count, feat_in);
        }
    } else {
        if (first_count > 0) {
            spmm256Seg<1, false><<<first_grid, block>>>(d_idx, d_val, vin, vout, first_start, first_len, first_row, first_count, feat_in);
        }
        if (rest_count > 0) {
            spmm256Seg<1, true><<<rest_grid, block>>>(d_idx, d_val, vin, vout, rest_start, rest_len, rest_row, rest_count, feat_in);
        }
    }
}