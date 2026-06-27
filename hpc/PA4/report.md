# Deepseek Sparse Attention Indexer Score BF16

王宇康 计51 2024010091

## 实现方法

### Triton 程序结构

最终实现采用 Triton Python 接口，`run_kernel` 接收评测程序传入的 CUDA tensor 和 Python 整数参数，并写入 `scores`，程序包含两个 `@triton.jit` kernel：
- `_sparse_indexer_64_kernel`：针对主要测试参数 `Hidx=64, Didx=128, PageSize=64` 的高性能路径
- `_generic_indexer_kernel`：针对其他参数的通用路径，计算方式与 baseline 基本相同

`run_kernel` 根据 `Hidx / Didx / PageSize` 选择路径，评测中的特定参数走高性能 kernel

### _sparse_indexer_64_kernel 线程分配

当 `Hidx=64, Didx=128, PageSize=64` 时，主 kernel 采用 page 为基本计算单元，一个 Triton program 处理一个 batch 中的 `PAGES_PER_PROG` 个 page，具体策略如下，因此大规模测例中一个 program 能覆盖 2 个 page，即 128 个 token row 

```python
pages_per_prog = 2 if MaxPages >= 32 else 1
BLOCK_M = pages_per_prog * 64
BLOCK_D = 64
```

因为同一个 batch 下 `q_idx[b,:,:]` 和 `w_idx[b,:]` 会被所有 page 复用，一次处理多个 page 可以减少 program 数量，并提高 Q 和权重的缓存复用率，没有继续增大到 4 个 page 是因为更大的 `acc` tile 会显著增加寄存器压力，实测反而可能降低 occupancy

grid 为 `grid = (ceil(MaxPages / pages_per_prog), B)`，其中第 0 维枚举 page group，第 1 维枚举 batch

### _sparse_indexer_64_kernel 实现思路

主计算将每个 page group 的 K 与当前 batch 的 Q 组织成矩阵相乘：

```
K_block[BLOCK_M, 128] * Q^T[128, 64] -> acc[BLOCK_M, 64]
```

实现中用 `tl.dot` 触发 BF16 Tensor Core，accumulator 使用 FP32：

```python
acc += tl.dot(k, tl.trans(q), out_dtype=tl.float32)
```

`Didx=128` 被拆成两个 `BLOCK_D=64` 的 K 方向 tile，是对于循环次数、load 开销和寄存器压力的折中选择

并且在 Triton kernel 中没有把完整 `dots` 写回全局内存，也没有保存到 shared memory，而是在 FP32 accumulator 上直接完成 ReLU、乘权重和 head 归约，从而避免了中间 `dots[64,64]` 的额外读写，比先存中间结果再启动或执行后处理更高效

```python
score = tl.sum(tl.maximum(acc, 0.0) * w[None, :], axis=1)
```

### 存储与访存优化

实现主要依赖 Triton 的寄存器 tile 和编译器调度，而不是手动管理 shared memory  

- `acc[BLOCK_M,64]` 常驻寄存器
- `w_idx[b,:]` 被加载为 FP32 向量后直接参与 epilogue
- `q_idx` 和 `w_idx` 在同一个 batch 的多个 page 中复用，因此 load 时使用 `eviction_policy="evict_last"`，尽量保留在 cache 中
- `k_idx_cache` 按 page 流式读取，使用 `eviction_policy="evict_first"`，减少对可复用 Q/W 数据的缓存污染。

通用路径的 fallback kernel 保持同样逻辑，但使用较小的 `BLOCK_M=16` 和动态 `BLOCK_H/BLOCK_D`，主要保证正确性

## 测例运行时间与加速比

|测试点| baseline | my_program | 加速比 |
| --- | --- | --- | --- |
| 5 | 4551 us | 10us | 455.10 |
| 7 | 18 ms | 48us | 375.00 |
| 10 | 67 ms | 182us | 368.13 |

## AI 工具使用

使用 chatgpt5.5 询问优化建议，部分代码实现以及 _sparse_indexer_64_kernel 函数调试