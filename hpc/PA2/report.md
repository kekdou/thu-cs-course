# 多源最短路径

王宇康 2024010091 计51

## 核函数

先总体介绍各个核函数作用，后文中详细说明程序实现方法

1. **padGraphKernel**: 将原来的图 graph 填充为新图 pad_graph，增加的部分填充 `INF = 0x3f3f3f3f` 
2. **unpadGraphKernel**: 将 pad_graph 中公共部分数据复制到 graph 中
3. **phase1Kernel**: 进行算法第一阶段，对角线上执行 floyd-warshall 算法
4. **phase2Kernel**: 进行算法第二阶段，对计算一行和一列
5. **phase3Kernel**: 进行算法第三阶段，从十字形扩散到整个图
6. **apsp_strategy**: 根据图的大小选择 block, thread 的数量
7. **apsp**: 程序执行逻辑

## 具体实现  

首先利用 `padGraphKernel` 将原图边长 n 扩充至 b_size 的整数倍 pad_n = $\lceil \frac{n}{b\_size} \rceil$ * b_size，得到新图 pad_graph，从而省去每一个块中复杂的边界判断，防止 warp 发散  

`padGraphKernel` 仅进行数据的读取和填充，因此使用 grid($\frac{pad\_n}{32}$, $\frac{pad\_n}{32}$), block(32, 32) 的配置 (达到单个 block 的最大线程数)，尽可能提高并发的线程数，提高带宽

然后根据 n 的大小选择不同的块大小 (B_SIZE * B_SIZE) 和每个块中的线程数 (T_SIZE * T_SZIE)，在综合考虑并行度、吞吐、访存速度等因素以及实验测试后，得出当 n $\le$ 1000 时，B_SIZE = 32，T_SIZE = 8，当 n $\gt$ 1000 时，B_SIZE = 64, T_SZIE = 16  性能表现最好  

在 pad_n / B_SIZE 次循环中，算法每次进行三个阶段：

```C++
dim3 block_dim(T_SIZE, T_SIZE);
dim3 p2_grid(num_blocks - 1, 2);
dim3 p3_grid(num_blocks - 1, num_blocks - 1);
for (int step = 0; step < num_blocks; step++) {
    phase1Kernel<B_SIZE, T_SIZE><<<1, block_dim>>>(pad_graph, pad_n, step);
    phase2Kernel<B_SIZE, T_SIZE><<<p2_grid, block_dim>>>(pad_graph, pad_n, step);
    phase3Kernel<B_SIZE, T_SIZE><<<p3_grid, block_dim>>>(pad_graph, pad_n, step);
}
```

三个阶段的配置如上代码所示

在三个阶段中：
- 均使用 `__launch_bounds__(T_SIZE * T_SIZE)` 修饰符分配寄存器数量，防止溢出严重影响性能  
- 用 `_ldg()` 读入 pad_graph 中的数据到 shared memory 中
- shared 数组 (B_kk[B_ZIZE][B_SZIE + 1]) 通过内存填充 (添加 padding) 来避免 bank conflict
- 通过 `#pragma unroll` 来尽可能展开循环  


利用向量化访存来提高数据吞吐  

```C++
int4* graph_int4 = (int4*)graph;
int pad_n_4 = pad_n >> 2;
```

并且尽量先将相关数据放入寄存器，减少在后续循环中的频繁调用

```C++
int target[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            target[i][j] = B_other[ty * 4 + i][tx * 4 + j];
        }
    }
```

最后将 pad_graph 中的数据通过 `unpadGraphKernel` 传回 graph
 
## 运行时间和加速比

| n | `apsp.cu` | `apspRef.cu` | 加速比 |
| --- | --- | --- | --- |
| 1000 | 1.692074 ms | 15.340865 ms | 9.066 |
| 2500 | 13.438595 ms | 379.655617 ms | 28.251 |
| 5000 | 72.718542 ms | 2994.376143 ms | 41.178 |
| 7500 | 231.706577 ms | 10076.740975 ms | 43.489 |
| 10000 | 538.659598 ms | 22912.675247 ms | 43.341 |