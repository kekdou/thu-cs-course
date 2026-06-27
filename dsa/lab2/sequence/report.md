# sequence

王宇康 计51 2024010091  

## C1

### grow()

以下约定 R = GrowthNum / GrowthDen

设置 new_capacity，如果原来 capacity_ 不为 0，则为 capacity_ * R，如果为 0，则为 INITIAL_CAPACITY  

并且，如果 new_capacity < capacity_，则将 new_capacity 重新设为 capacity_ + 1，保证扩容后容量变大  

最后利用 reserve() 实现重新分配内存操作  

### shrink()

设置 threshold 为 capacity_ 的 $\frac{1}{R^2}$，如果 size_ 比 threshold 大，说明不需要缩容，直接返回，如果比 threshold 小，则进行缩容操作  

设置 new_capacity 为 capacity_ 的 $\frac{1}{R}$，并且同样，如果比 INITIAL_CAPACITY 小则重新设为 INITIAL_CAPACITY，如果比size_ 小则设为 size_ + 1  

然后利用 new_capacity 开辟内存，拷贝数据，更新 capacity_  

## T1
 
**结论**：大体符合理论差距  

对于扩容系数为 m 的 vector，push_back 的均摊时间复杂度为 $O(\frac{m}{m - 1})$，均为 $O(1)$，仅在常数上有所差异  
理论上讲，m 越大，操作的时间越小，但受系统内存分配，缓存等各种因素影响，也存在一些例外
最终从测试结果来看，不同数据规模的操作时间基本相同，为 $O(1)$ 且总体符合理论规律  

**数据**：
1. 采用大数的数据类型 `using DataType = BigData`
2. 产生 n = [10000, 100000, 1000000] 的 `append.txt`，用于测试 grow()
3. 先利用 n = [1000, 10000, 100000, 1000000] 的 `append.txt` 初始化数组，然后额外实现了 `pop.txt`，连续进行相应次数的 pop_back 操作，仅计算 pop_back 的时间，用于测试 shrink()

**结果**：

```
=== append (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
std::vector                 1.01       101.3
Vector<1.5x>                1.16       115.7
Vector<2x>                  0.43        42.9
Vector<4x>                  0.57        56.7
Vector<8x>                  0.12        11.9

=== pop (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
std::vector                 0.00         0.4
Vector<1.5x>                0.11        11.0
Vector<2x>                  0.16        16.3
Vector<4x>                  0.04         3.9
Vector<8x>                  0.02         2.5
```

```
=== append (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
std::vector                 7.97        79.7
Vector<1.5x>               19.46       194.6
Vector<2x>                  5.48        54.8
Vector<4x>                 13.10       131.0
Vector<8x>                  1.99        19.9

=== pop (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
std::vector                 0.12         1.2
Vector<1.5x>                5.38        53.8
Vector<2x>                  1.73        17.3
Vector<4x>                  1.91        19.1
Vector<8x>                  0.23         2.3
```

```
=== append (n=1000000) ===
Container               Time(ms)       ns/op
--------------------------------------------
std::vector                78.71        78.7
Vector<1.5x>              137.79       137.8
Vector<2x>                 84.59        84.6
Vector<4x>                 54.90        54.9
Vector<8x>                 45.02        45.0

=== pop (n=1000000) ===
Container               Time(ms)       ns/op
--------------------------------------------
std::vector                 0.88         0.9
Vector<1.5x>               83.44        83.4
Vector<2x>                 34.97        35.0
Vector<4x>                 14.92        14.9
Vector<8x>                  6.36         6.4
```

## C2

### PtrLinkedList

普通的双向链表，每个节点储存两个节点的指针  

每次插入和删除都进行内存的分配或者删除，然后更新目标索引节点的前后指针以及链表的 size_

### PoolPtrLinkedList

在链表初始化时先分配一个 POOL_SIZE 的大内存，记录使用过程中的 free_head_，做到在插入和删除过程中省去频繁的动态内存操作，节约时间  

#### insert()

先利用 alloc_node() 得到第一个空闲位置的指针，同时更新 free_head_ 等  

然后将空间位置赋值为目标 val  

再利用 get_node(index) 找到目标位置的节点，即为新节点的下一个节点 

更新节点之间的前后指针关系，更新 size_  

#### remove()

利用 get_node(index) 找到目标节点的位置  

更新目标位置前后指针的连接关系，然后用 free_node() 将目标位置标记为空闲节点，更新 size_

### ArrayLinkedList

实现原理与 PoolPtrLinkedList 类似，只不过将每个储存的前后指针关系储存在 prev_ 和 next_ 数组中，以索引形式呈现

#### insert()

利用 alloc_node() 获取第一个空闲位置的索引，在 val_ 数组中更新 val   

用 get_node_idx() 获取第 index 个节点的位置，更新之间的 next_ 和 prev_ 关系  

更新 size_

#### remove()

用 get_node_idx(index) 获取删除节点的索引，然后更新 prev_ 和 next_  

最后 free_node()，更新 free_head_ 以及 size_ 等  

## T2

**结论**：

理论上而言，不同链表的 insert 和 remove 时间复杂度均为 $O(n)$，包括查找和插入删除的时间  

但由于计算机 cpu 缓存命中与内存分配的相关机制，不同实现方式的链表性能差异明显，在绝大多数数据规模下，$ArrayLinkedList \gt PoolPtrLinkedList \gt PtrLinkedList$  

- **PtrLinkedList**：每次插入和删除都需要对内存进行分配和释放，严重浪费时间，并且内存分散，每次 node = node->next 都可能触发 cache miss，因此表现最差
- **PoolPtrLinkedList**：由于预先分配了一大块内存作为 Pool，省去了 new 和 delete 的时间，但由于数据规模的不断增长，相邻节点在物理内存上的分布依然杂乱分散，在查找时依然无法解决 cache miss 问题，但性能比 PtrLinkedList 有所提升
- **ArrayLinkedList**：所有节点位置用索引在数组中表示，空间分布与动态内存操作都得到优化，因此性能最好

**数据**：  
1. 选用 int 数据类型 `using DataType = int`
2. 生成 n = [1000, 10000, 100000] 的 `random_insert.txt` 测试 insert()
3. 额外实现 `random_remove.txt`，对长度为 n 的链表进行删除，每次删除的位置为 [0, size) 中的随机数，测试 remove() 的性能

**结果**：

```
=== random_insert (n=1000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               0.26       262.0
PoolPtrLinkedList           0.22       219.4
ArrayLinkedList             0.25       254.3

=== random_remove (n=1000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               0.24       237.3
PoolPtrLinkedList           0.21       212.0
ArrayLinkedList             0.27       268.1
```

```
=== random_insert (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList              67.40      6740.2
PoolPtrLinkedList          62.17      6216.7
ArrayLinkedList            30.16      3015.7

=== random_remove (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList              68.37      6836.6
PoolPtrLinkedList          67.43      6743.3
ArrayLinkedList            30.04      3003.7
```

```
=== random_insert (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList           27613.54    276135.4
PoolPtrLinkedList       21321.57    213215.7
ArrayLinkedList          7067.68     70676.8

=== random_remove (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList           32805.16    328051.6
PoolPtrLinkedList       22898.61    228986.1
ArrayLinkedList         10681.09    106810.9
```

## T3

**结论**：是 $O(1)$ 操作 

在 `linked_list.hpp` 中分别实现了三种链表的 push_back(), push_front(), pop_back(), pop_front() 方法  

直接利用 head_, tail_ 方法操作，避免 insert 和 remove 多余的搜索操作  

**数据**：
1. 额外实现 push_front.txt, pop_front.txt, pop_back.txt 测例  
2. 在 n = [10000, 100000, 1000000] 数据规模下对四种头尾操作进行测试

**结果**：

```
=== append (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               0.22        22.4
PoolPtrLinkedList           0.06         6.4
ArrayLinkedList             0.05         4.8

=== pop_back (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               0.01         0.6
PoolPtrLinkedList           0.02         1.5
ArrayLinkedList             0.02         2.1

=== pop_front (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               0.00         0.5
PoolPtrLinkedList           0.01         0.9
ArrayLinkedList             0.01         0.6

=== push_front (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               0.08         8.3
PoolPtrLinkedList           0.03         3.4
ArrayLinkedList             0.03         2.7
```

```
=== append (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               1.80        18.0
PoolPtrLinkedList           0.44         4.4
ArrayLinkedList             0.46         4.6

=== pop_back (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               0.08         0.8
PoolPtrLinkedList           0.26         2.6
ArrayLinkedList             0.22         2.2

=== pop_front (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               0.09         0.9
PoolPtrLinkedList           0.12         1.2
ArrayLinkedList             0.11         1.1

=== push_front (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               0.72         7.2
PoolPtrLinkedList           0.19         1.9
ArrayLinkedList             0.27         2.7
```

```
=== append (n=1000000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList              15.80        15.8
PoolPtrLinkedList           2.82         2.8
ArrayLinkedList             4.41         4.4

=== pop_back (n=1000000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               0.99         1.0
PoolPtrLinkedList           2.95         2.9
ArrayLinkedList             1.61         1.6

=== pop_front (n=1000000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               0.95         0.9
PoolPtrLinkedList           0.93         0.9
ArrayLinkedList             0.97         1.0

=== push_front (n=1000000) ===
Container               Time(ms)       ns/op
--------------------------------------------
PtrLinkedList               8.38         8.4
PoolPtrLinkedList           7.84         7.8
ArrayLinkedList             3.42         3.4
```

## C3

由于 $xor\_ptr = prev \oplus next$ 则可以以利用 $xor\_ptr \oplus prev = next$ 以及 $xor\_ptr \oplus next = prev$ 来获取相邻节点指针  

同时给出了 get_node() 接口  

### insert()

首先利用 get_node(index) 定位到插入位置的左右节点  

new 一个新节点，并初始化其 xor_ptr, 利用上面异或的关系，更新 prev_node 和 next_node 的 xor_ptr  

同时对 head_ 和 tail_ 进行边界检查，然后更新 size_ 即可  

### remove()

实现原理类似，利用 get_node() 定位，更新相关节点的 xor_ptr  

进行头尾指针的边界检查，delete 目标节点，更新 size_ 即可  

## C4

### split_chunk()

先调用 insert_chunk_after() 在当前 chunk 后新建一个 new_chunk  

然后使用 std::memmove 将 原来 chunk 从中间位置(ChunkSize / 2) 开始的后半部分数据拷贝到 new_chunk 头部  

更新 chunk 和 new_chunk 的 size  

### try_merge()

先检查当前 chunk 和 下一个 next_chunk 的大小和是否符合合并要求，即 chunk->size + next_chunk->size $\le$ ChunkSize  

如果符合进行接下来的操作，否则直接 return  

使用 std::memmove 将 next_chunk 中的数据拷贝到当前 chunk 的末尾  

更新 chunk 的 size，调用 remove_chunk() 删除 next_chunk  

## T4

**结论**：

1. 尾部操作  
   - 无论是 UnrollLinkedList<64> 还是 UnrollLinkedList<256> 在插入和删除尾部元素时速度都极快，$O(1)$
   - 因为尾部增删，本质上就是在最后一个 chunk 的内置数组末尾进行操作，性能几乎就是 vector
2. 头部操作
   - 尽管二者时间都很快，为 $O(1)$ 但是较小的 chunk_size 会比较大的更快，也就是 <64> 快于 <256>
   - 这是因为在头部操作时，必须把该 chunk 内已有的元素向后或向前移动，时间复杂度受限于块大小 Chunk_Size，$O(Chunk_Size)$，所以块小的时间会快一些
3. 随机增删
   - 理论时间复杂度为 $O(N / Chunk_SIze) + O(Chunk_Size)$ 
   - 但块大的链表在规模大的数据时表现的会更好，同样也受到 cpu 缓存命中，空间分布等影响
4. 遍历与查询
   - 同样块大的链表表现会比小的好，原因与上同

**数据**：

1. 生成了 n = [10000, 100000, 1000000] 的 append.txt, pop_back.txt, push_front.txt, pop_back.txt 测试首尾操作的性能
2. 生成了 n = [1000, 10000, 100000] 的 random_insert.txt, random_remove.txt 测试随机 insert 和 remove 操作
3. 进行 n = [1000, 10000, 100000], m = 1000 的遍历测试，测试 traverse 的性能
4. 重新实现了 gen_query，单独测试 query 的时间，而不计入 build 的时间，在 n = [10000, 100000, 1000000] 的规模下进行，测试 query 操作

**结果**：

1. push_back()

```
=== append (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.03         2.6
UnrollLinkedList<256>       0.01         1.5

=== append (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.52         5.2
UnrollLinkedList<256>       0.21         2.1

=== append (n=1000000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        3.10         3.1
UnrollLinkedList<256>       2.69         2.7
```

2. pop_back()

```
=== pop_back (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.01         1.1
UnrollLinkedList<256>       0.01         0.8

=== pop_back (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.11         1.1
UnrollLinkedList<256>       0.09         0.9

=== pop_back (n=1000000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        1.11         1.1
UnrollLinkedList<256>       0.97         1.0
```

3. push_front()

```
=== push_front (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.06         5.6
UnrollLinkedList<256>       0.09         9.2

=== push_front (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.59         5.9
UnrollLinkedList<256>       0.90         9.0

=== push_front (n=1000000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        5.32         5.3
UnrollLinkedList<256>       7.31         7.3
```

4. pop_front

```
=== pop_front (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.05         4.5
UnrollLinkedList<256>       0.08         8.2

=== pop_front (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.45         4.5
UnrollLinkedList<256>       0.71         7.1

=== pop_front (n=1000000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        4.56         4.6
UnrollLinkedList<256>       6.68         6.7
```

5. insert()

```
=== random_insert (n=1000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.02        20.0
UnrollLinkedList<256>       0.02        15.5

=== random_insert (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.68        68.1
UnrollLinkedList<256>       0.28        28.0

=== random_insert (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>      178.55      1785.5
UnrollLinkedList<256>      21.36       213.6
```

6. remove()

```
=== random_remove (n=1000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.02        20.8
UnrollLinkedList<256>       0.02        15.4

=== random_remove (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.71        70.6
UnrollLinkedList<256>       0.30        29.6

=== random_remove (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>      209.94      2099.4
UnrollLinkedList<256>      25.43       254.3
```

7. traverse

```
=== traverse (n=1000, m=1000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.76         0.8
UnrollLinkedList<256>       0.37         0.4

=== traverse (n=10000, m=1000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        5.49         0.5
UnrollLinkedList<256>       3.55         0.4

=== traverse (n=100000, m=1000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>       45.52         0.5
UnrollLinkedList<256>      35.96         0.4
```

8. query

```
=== query (n=10000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.01         0.6
UnrollLinkedList<256>       0.00         0.5

=== query (n=100000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        0.06         0.6
UnrollLinkedList<256>       0.05         0.5

=== query (n=1000000) ===
Container               Time(ms)       ns/op
--------------------------------------------
UnrollLinkedList<64>        1.14         1.1
UnrollLinkedList<256>       1.34         1.3
```

## E

- **E1**:

假设初始容量为 1，每次容量满时按 $k$ 倍扩容($k \gt 1$)  

不断 push_back 直到元素总数为 $N$ ，此时经历了 $m$ 次扩容，有 $k^m \approx N$  

基础的插入需要 N 次操作，拷贝需要 $\sum_{i = 0}^{m - 1} k^{i} = \frac{k^{m} - 1}{k - 1} \approx \frac{N}{k - 1}$  

因此总共为 $N + \frac{N}{k - 1} = \frac{k}{k - 1} N$，由此均摊时间复杂度为 $O(\frac{k}{k - 1}) = O(1)$  

- **E2**:  

GCC 使用 2 倍扩容， MSVC 使用 1.5 倍扩容  

扩容策略不太一致，标准库中在执行删除操作时不会主动缩容 shrink，只能手动缩容  

- **E3**: 

List 在动态操作上理论比 vector 要好，但在测试中几乎没有出现 list 在某种操作上比 vector 要好的情况  

但也存在，如头部操作 push_front(), pop_front()，均是 $O(1)$，比 vector 性能要好

一般来说，仅对头部操作的情况较少，并且在计算机缓存机制下，list $O(N)$ 的指针跳转会引发大量 cache miss，还有频繁调用 new 和 delete 的问题，导致性能不如 vector  

- **E4**：

不能，只有一个节点的 xor_ptr 无法计算出相邻节点的指针，因此必须两个相邻节点才行  

- **E5**：

不太合适  

目前使用的是 SoA 布局，在链表的遍历和增删时，不可避免地需要同时访问同一个节点的 val、prev 和 next，但在这种布局下，三个数据分布在三块完全独立的区域，因此对缓冲不友好  

可以使用 AoS 布局

```C++
struct Node {
   T val;
   int prev_idx;
   int next_idx;
}
Node* pool_
```

- **E6**：

python 中的 collections.deque 底层是一个双向分块链表，类似 UnrollLinkedList 的结构  

正如 T4 的结果所示，这种结构拥有 $O(1)$ 的两端操作，并且一定程度上解决了 list cache 和空间不友好的问题  

- **E7**:

remove_at_unordered 与 swap_remove 通过将删除目标与数组最后一个元素交换，再执行pop_back() 操作，完成删除  

预期时间复杂度为 $O(1)$，没有多余的数据搬运操作  

- **E8**:
 
存在，显著

采用 BigData 后，vector 的性能差距变大，由于拷贝的负担增大  

但是对于链表来说，测试了头尾操作以及随机插入和删除，PtrLinkedList, PoolPtrLinkedList, ArrayLinkedList 的各项操作性能几乎没有变化，但 Xor_List 的插入元素相关操作性能变化极大，删除略微变化  

猜测也是由于内部 vector 引起

- **E9**:

1. 尽量使用内存连续的数据结构
2. 如果不在乎数据的循序，swap_remove 是很好的选择
3. 单端操作用 vector 或者 queue，双端操作用 deque