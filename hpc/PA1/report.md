# 奇偶排序

王宇康 2024010091 计51  

## 实验代码  

简要说明在代码中注释呈现  

```C++
// 基数排序
static void radix_sort(float* arr, float* temp, size_t len) {
  uint32_t* src = reinterpret_cast<uint32_t*>(arr);
  uint32_t* dst = reinterpret_cast<uint32_t*>(temp);
  for (size_t i = 0; i < len; ++i) {
    uint32_t u = src[i];
    src[i] = (u >> 31) ? ~u : (u | 0x80000000);
  }
  for (int shift = 0; shift < 32; shift += 8) {
    uint32_t count[256] = {0};
    for (size_t i = 0; i < len; ++i) {
      count[(src[i] >> shift) & 0xFF]++;
    }
    uint32_t pos[256];
    pos[0] = 0;
    for (int i = 1; i < 256; ++i) {
        pos[i] = pos[i - 1] + count[i - 1];
    }
    for (size_t i = 0; i < len; ++i) {
      dst[pos[(src[i] >> shift) & 0xFF]++] = src[i];
    }
    uint32_t* tmp = src;
    src = dst;
    dst = tmp;
  }
  for (size_t i = 0; i < len; ++i) {
    uint32_t u = src[i];
    src[i] = (u >> 31) ? (u & 0x7FFFFFFF) : ~u;
  }
}

void Worker::sort() {
  if (out_of_range || block_len == 0) {
    return;
  }
  size_t b_size = (n + nprocs - 1) / nprocs;
  int active_procs = (n + b_size - 1) / b_size;
  float* bufferB = new float[b_size];
  float* recv_data = new float[b_size];
  // 如果每个进程分配的 block_size > 10000 则采用基数排序，否则直接使用 std::sort
  if (block_len > 10000) {
    radix_sort(data, bufferB, block_len);
  } else {
    std::sort(data, data + block_len);
  }
  // 采用双缓冲区储存数据
  float* src = data;
  float* dst = bufferB;
  // 固定跑满 nproces 轮，结果一定为有序
  for (int shift = 0; shift < nprocs; shift++) {
    // 判断奇偶数轮次，选择相应邻居位置
    int neighbor = (shift & 1) ? (rank & 1 ? rank + 1 : rank - 1) : (rank & 1 ? rank - 1 : rank + 1);
    if (neighbor >= 0 && neighbor < active_procs) {
      size_t neighbor_len = (neighbor == active_procs - 1) ? (n - neighbor * b_size) : b_size;

      float my_pivot, recv_pivot;
      if (rank < neighbor) {
          my_pivot = src[block_len - 1];
      } else {
          my_pivot = src[0];
      }
      // 先 Sendrecv 一次最小值与最大值，比较后判断后续是否需要进一步交换
      MPI_Sendrecv(&my_pivot, 1, MPI_FLOAT, neighbor, 0,
                   &recv_pivot, 1, MPI_FLOAT, neighbor, 0,
                   MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      bool need_exchange = 0;
      if (rank < neighbor) {
        if (my_pivot > recv_pivot) {
          need_exchange = 1; 
        }
      } else {
        if (my_pivot < recv_pivot) {
          need_exchange = 1;
        }
      }
      // 如果需要交换
      if (need_exchange) {
        MPI_Sendrecv(src, block_len, MPI_FLOAT, neighbor, 1, 
                     recv_data, neighbor_len, MPI_FLOAT, neighbor, 1, 
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (rank < neighbor) {
          // 查找需要归并的本地数组和邻居数组数量
          size_t left = (block_len > neighbor_len) ? (block_len - neighbor_len) : 0;
          size_t right = block_len;
          while (left < right) {
            size_t mid = (left + right + 1) >> 1;
            size_t j_index = block_len - mid;
            float i_val = (mid == 0) ? -1e38f : src[mid - 1];
            float j_val = (j_index == neighbor_len) ? 1e38f : recv_data[j_index];
            if (i_val <= j_val) {
              left = mid;
            } else {
              right = mid - 1;
            }
          }
          size_t i_len = left;
          size_t j_len = block_len - left;
          size_t i = 0, j = 0, k = 0;
          // 归并
          while (i < i_len && j < j_len) {
            if (src[i] <= recv_data[j]) {
              dst[k++] = src[i++];
            } else {
              dst[k++] = recv_data[j++];
            }
          }
          while (i < i_len) dst[k++] = src[i++];
          while (j < j_len) dst[k++] = recv_data[j++];
        } else {
          size_t left = 0;
          size_t right = std::min(block_len, neighbor_len);
          while (left < right) {
            size_t mid = (left + right + 1) >> 1;
            size_t j_index = neighbor_len - mid;
            float i_val = (mid == 0) ? -1e38f : src[mid - 1];
            float j_val = (j_index == neighbor_len) ? 1e38f : recv_data[j_index];
            if (i_val <= j_val) {
              left = mid;
            } else {
              right = mid - 1;
            }
          }
          size_t i = left;
          size_t j = neighbor_len - left;
          size_t k = 0;
          while (i < block_len && j < neighbor_len) {
            if (src[i] <= recv_data[j]) {
              dst[k++] = src[i++];
            } else {
              dst[k++] = recv_data[j++];
            }
          }
          while (i < block_len) dst[k++] = src[i++];
          while (j < neighbor_len) dst[k++] = recv_data[j++];
        }
        std::swap(src, dst);
      }
    }
  }
  // 将最终数据 copy 到 data
  if (src != data) {
    memcpy(data, src, block_len * sizeof(float));
  }
  delete[] recv_data;
  delete[] bufferB;
}
```

## 我做的优化

1. 采用基数排序，利用额外的空间将排序的时间压缩到 O(n), 并且在 block_size 较大的情况下使用（本实验中为 10000），保证排序带来的收益远大于浮点数处理时间
2. 固定跑满 nprocs 轮次，省去判断排序是否完成的 allreduce 开销，因为测得本次实验，一般情况下 allreduce 的开销大于一轮计算的开销  
3. 每次交换数据前，先单独 Sendrecv 每个进程的最值（最大或最小，取决于奇偶轮与相对位置），如果左边的最大值小于右边的最小值，则说明已经有序，无需进行后续的操作，并且这个检查配合第 2 点完全可以实现 allreduce 提前结束的功能  
4. 在归并过程先利用二分查找，得出左右两个数组需要归并的数量，从而减少归并时频繁的条件判断，再配合 O3 优化，效果提升明显
5. 采用双指针交换的方法存储中间数据，在每轮循环中交换 src 和 dst，从而避免频繁动态分配内存，造成巨大开销
6. 在编译时采用 O3 优化，并且使用 --cpu-bind=cores --exclusive，将进程绑定核，并且独占

采用以上方法，效果会比单纯采用 sort 以及 allreduce 和 动态分配内存等提升 40% 以上

## 运行时间及加速比

在助教的 100000000.dat 测试中

| 进程 | 时间 | 加速比 |
| --- | --- | --- |
| 1 * 1 | 3289.718000 ms | 1 |
| 1 * 2 | 1936.223000 ms | 1.69904 |
| 1 * 4 | 1196.268000 ms | 2.74998 |
| 1 * 8 | 828.933000 ms | 3.96862 |
| 1 * 16 | 655.830000 ms | 5.01611 |
| 2 * 16 | 487.853000 ms | 6.74326 |
