// DO NOT MODIFY THE INCLUDES
#include "apsp.h"
#include <cuda_runtime.h>

#define INF 0x3f3f3f3f
#define SHARED_PADDING 1
#define PAD_BLOCK_SIZE 32

__global__ void padGraphKernel(const int* src, int* dst, int n, int pad_n) {
    int c = blockIdx.x * blockDim.x + threadIdx.x;
    int r = blockIdx.y * blockDim.y + threadIdx.y;
    if (r < n && c < n) {
        dst[r * pad_n + c] = __ldg(&src[r * n + c]);    // 技巧：使用 __ldg 读取只读数据
    } else {
        dst[r * pad_n + c] = (r == c) ? 0 : INF;
    }
}

__global__ void unpadGraphKernel(const int* src, int* dst, int n, int pad_n) {
    int c = blockIdx.x * blockDim.x + threadIdx.x;
    int r = blockIdx.y * blockDim.y + threadIdx.y;
    if (r < n && c < n) {
        dst[r * n + c] = __ldg(&src[r * pad_n + c]);
    }
}


template <int B_SIZE, int T_SIZE>
__global__ void __launch_bounds__(T_SIZE * T_SIZE) phase1Kernel(int* graph, int pad_n, int step) {
    __shared__ int B_kk[B_SIZE][B_SIZE + SHARED_PADDING];

    int tx = threadIdx.x;
    int ty = threadIdx.y;

    int4* graph_int4 = (int4*)graph;
    int pad_n_4 = pad_n >> 2;

    #pragma unroll
    for (int i = 0; i < 4; i++) {
        int row = ty * 4 + i;
        int4 val = __ldg(&graph_int4[(step * B_SIZE + row) * pad_n_4 + (step * T_SIZE + tx)]);
        B_kk[row][tx * 4 + 0] = val.x;
        B_kk[row][tx * 4 + 1] = val.y;
        B_kk[row][tx * 4 + 2] = val.z;
        B_kk[row][tx * 4 + 3] = val.w;
    }
    __syncthreads();
    
    #pragma unroll
    for (int k = 0; k < B_SIZE; k++) {
        #pragma unroll
        for (int i = 0; i < 4; i++) {
            int row = ty * 4 + i;
            int ik = B_kk[row][k];
            #pragma unroll
            for (int j = 0; j < 4; j++) {
                int col = tx * 4 + j;
                B_kk[row][col] = min(B_kk[row][col], ik + B_kk[k][col]);
            }
        }
        __syncthreads(); 
    }
    
    #pragma unroll
    for (int i = 0; i < 4; i++) {
        int row = ty * 4 + i;
        int4 val = {B_kk[row][tx * 4 + 0], B_kk[row][tx * 4 + 1], B_kk[row][tx * 4 + 2], B_kk[row][tx * 4 + 3]};
        graph_int4[(step * B_SIZE + row) * pad_n_4 + (step * T_SIZE + tx)] = val;
    }
}

template <int B_SIZE, int T_SIZE>
__global__ void __launch_bounds__(T_SIZE * T_SIZE) phase2Kernel(int* graph, int pad_n, int step) {
    int b_block = blockIdx.x + (blockIdx.x >= step);
    
    __shared__ int B_kk[B_SIZE][B_SIZE + 1];    
    __shared__ int B_other[B_SIZE][B_SIZE + 1]; 
    
    int tx = threadIdx.x;
    int ty = threadIdx.y;
    
    int4* graph_int4 = (int4*)graph;
    int pad_n_4 = pad_n >> 2;
    
    #pragma unroll
    for (int i = 0; i < 4; i++) {
        int row = ty * 4 + i;
        int4 val_kk = __ldg(&graph_int4[(step * B_SIZE + row) * pad_n_4 + (step * T_SIZE + tx)]);
        B_kk[row][tx * 4 + 0] = val_kk.x;
        B_kk[row][tx * 4 + 1] = val_kk.y;
        B_kk[row][tx * 4 + 2] = val_kk.z;
        B_kk[row][tx * 4 + 3] = val_kk.w;
    }
    
    if (blockIdx.y == 0) {
        #pragma unroll
        for (int i = 0; i < 4; i++) {
            int row = ty * 4 + i;
            int4 val_other = __ldg(&graph_int4[(step * B_SIZE + row) * pad_n_4 + (b_block * T_SIZE + tx)]);
            B_other[row][tx * 4 + 0] = val_other.x;
            B_other[row][tx * 4 + 1] = val_other.y;
            B_other[row][tx * 4 + 2] = val_other.z;
            B_other[row][tx * 4 + 3] = val_other.w;
        }
        __syncthreads();
        
        int target[4][4];
        #pragma unroll
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                target[i][j] = B_other[ty * 4 + i][tx * 4 + j];
            }
        }
        
        #pragma unroll
        for (int k = 0; k < B_SIZE; k++) {
            int kj[4] = {B_other[k][tx * 4 + 0], B_other[k][tx * 4 + 1], B_other[k][tx * 4 + 2], B_other[k][tx * 4 + 3]};
            #pragma unroll
            for (int i = 0; i < 4; i++) {
                int ik = B_kk[ty * 4 + i][k];
                target[i][0] = min(target[i][0], ik + kj[0]);   // 对 target 写入，防止读写冲突
                target[i][1] = min(target[i][1], ik + kj[1]);
                target[i][2] = min(target[i][2], ik + kj[2]);
                target[i][3] = min(target[i][3], ik + kj[3]);
            }
        }
        
        #pragma unroll
        for (int i = 0; i < 4; i++) {
            int row = ty * 4 + i;
            int4 val = {target[i][0], target[i][1], target[i][2], target[i][3]};
            graph_int4[(step * B_SIZE + row) * pad_n_4 + (b_block * T_SIZE + tx)] = val;
        }
        
    } else {
        #pragma unroll
        for (int i = 0; i < 4; i++) {
            int row = ty * 4 + i;
            int4 val_other = __ldg(&graph_int4[(b_block * B_SIZE + row) * pad_n_4 + (step * T_SIZE + tx)]);
            B_other[row][tx * 4 + 0] = val_other.x;
            B_other[row][tx * 4 + 1] = val_other.y;
            B_other[row][tx * 4 + 2] = val_other.z;
            B_other[row][tx * 4 + 3] = val_other.w;
        }
        __syncthreads();
        
        int target[4][4];
        #pragma unroll
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                target[i][j] = B_other[ty * 4 + i][tx * 4 + j];
            }
        }

        #pragma unroll
        for (int k = 0; k < B_SIZE; k++) {
            int kj[4] = {B_kk[k][tx * 4 + 0], B_kk[k][tx * 4 + 1], B_kk[k][tx * 4 + 2], B_kk[k][tx * 4 + 3]};
            #pragma unroll
            for (int i = 0; i < 4; i++) {
                int ik = B_other[ty * 4 + i][k];
                target[i][0] = min(target[i][0], ik + kj[0]);
                target[i][1] = min(target[i][1], ik + kj[1]);
                target[i][2] = min(target[i][2], ik + kj[2]);
                target[i][3] = min(target[i][3], ik + kj[3]);
            }
        }

        #pragma unroll
        for (int i = 0; i < 4; i++) {
            int row = ty * 4 + i;
            int4 val = {target[i][0], target[i][1], target[i][2], target[i][3]};
            graph_int4[(b_block * B_SIZE + row) * pad_n_4 + (step * T_SIZE + tx)] = val;
        }
    }
}

template <int B_SIZE, int T_SIZE>
__global__ void __launch_bounds__(T_SIZE * T_SIZE) phase3Kernel(int* graph, int pad_n, int step) {
    int bx = blockIdx.x + (blockIdx.x >= step);
    int by = blockIdx.y + (blockIdx.y >= step);

    int tx = threadIdx.x;
    int ty = threadIdx.y;

    __shared__ int B_ik[B_SIZE][B_SIZE + SHARED_PADDING];
    __shared__ int B_kj[B_SIZE][B_SIZE + SHARED_PADDING];

    int4* graph_int4 = (int4*)graph;
    int pad_n_4 = pad_n >> 2;

    #pragma unroll
    for (int i = 0; i < 4; i++) {
        int row = ty * 4 + i;
        int4 val_ik = __ldg(&graph_int4[(by * B_SIZE + row) * pad_n_4 + (step * T_SIZE + tx)]);
        B_ik[row][tx * 4 + 0] = val_ik.x;
        B_ik[row][tx * 4 + 1] = val_ik.y;
        B_ik[row][tx * 4 + 2] = val_ik.z;
        B_ik[row][tx * 4 + 3] = val_ik.w;
        
        int4 val_kj = __ldg(&graph_int4[(step * B_SIZE + row) * pad_n_4 + (bx * T_SIZE + tx)]);
        B_kj[row][tx * 4 + 0] = val_kj.x;
        B_kj[row][tx * 4 + 1] = val_kj.y;
        B_kj[row][tx * 4 + 2] = val_kj.z;
        B_kj[row][tx * 4 + 3] = val_kj.w;
    }

    __syncthreads();

    int target[4][4];
    #pragma unroll
    for (int i = 0; i < 4; i++) {
        int row = ty * 4 + i;
        int4 val_target = __ldg(&graph_int4[(by * B_SIZE + row) * pad_n_4 + (bx * T_SIZE + tx)]);
        target[i][0] = val_target.x;
        target[i][1] = val_target.y;
        target[i][2] = val_target.z;
        target[i][3] = val_target.w;
    }

    #pragma unroll
    for (int k = 0; k < B_SIZE; k++) {
        int kj[4] = {B_kj[k][tx * 4 + 0], B_kj[k][tx * 4 + 1], B_kj[k][tx * 4 + 2], B_kj[k][tx * 4 + 3]};
        #pragma unroll
        for (int i = 0; i < 4; i++) {
            int ik = B_ik[ty * 4 + i][k];
            target[i][0] = min(target[i][0], ik + kj[0]);
            target[i][1] = min(target[i][1], ik + kj[1]);
            target[i][2] = min(target[i][2], ik + kj[2]);
            target[i][3] = min(target[i][3], ik + kj[3]);
        }
    }

    #pragma unroll
    for (int i = 0; i < 4; i++) {
        int row = ty * 4 + i;
        int4 val = {target[i][0], target[i][1], target[i][2], target[i][3]};
        graph_int4[(by * B_SIZE + row) * pad_n_4 + (bx * T_SIZE + tx)] = val;
    }
}

template <int B_SIZE, int T_SIZE>
void apsp_strategy(int n, int *pad_graph, int pad_n) {
    int num_blocks = pad_n / B_SIZE;
    dim3 block_dim(T_SIZE, T_SIZE);
    dim3 p2_grid(num_blocks - 1, 2);
    dim3 p3_grid(num_blocks - 1, num_blocks - 1);
    
    for (int step = 0; step < num_blocks; step++) {
        phase1Kernel<B_SIZE, T_SIZE><<<1, block_dim>>>(pad_graph, pad_n, step);
        phase2Kernel<B_SIZE, T_SIZE><<<p2_grid, block_dim>>>(pad_graph, pad_n, step);
        phase3Kernel<B_SIZE, T_SIZE><<<p3_grid, block_dim>>>(pad_graph, pad_n, step);
    }
}

void apsp(int n, /* device */ int *graph) {
    int b_size = (n <= 1000) ? 32 : 64;
    int pad_n = ((n + b_size - 1) / b_size) * b_size;
    int* pad_graph = nullptr;
    cudaMalloc(&pad_graph, pad_n * pad_n * sizeof(int));
    
    dim3 pad_block(PAD_BLOCK_SIZE, PAD_BLOCK_SIZE);
    dim3 pad_grid(pad_n / PAD_BLOCK_SIZE, pad_n / PAD_BLOCK_SIZE);
    padGraphKernel<<<pad_grid, pad_block>>>(graph, pad_graph, n, pad_n);
    
    if (n <= 1000) {
        apsp_strategy<32, 8>(n, pad_graph, pad_n);
    } else {
        apsp_strategy<64, 16>(n, pad_graph, pad_n);
    }
    
    unpadGraphKernel<<<pad_grid, pad_block>>>(pad_graph, graph, n, pad_n);
    cudaFree(pad_graph);
}
