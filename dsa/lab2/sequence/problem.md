# LAB2: Sequence

本次实现要求补全多种线性数据结构的实现(**C** 任务),
在不同的场景中测试(**T** 任务), 并回答思考题(**E** 任务).

针对测试任务, 你需要提供能够支持你结论的性能数据. 没有明显差距也是一种结论, 
但是对于与你的直觉相悖的情况, 请考虑是否是测试数据量过小或其他问题.

我们鼓励在 **T/E** 任务中使用 LLM 探究更深层的性能影响因素.
**C** 任务可以使用 LLM 辅助问答, 但是请**自行**完成代码补全.


## 接口与模板

### DequeInterface

所有数据结构继承 `DequeInterface<Derived>` ([CRTP 模式](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern), 可以理解为静态基类, 与使用 vtable 的动态接口不同)，需要实现以下基本操作：

```cpp
void insert(int idx, int val);  // 在位置 idx 插入 val
void remove(int idx);           // 删除位置 idx 的元素
int query(int idx);             // 查询位置 idx 的元素
std::size_t size();             // 返回元素数量
Iterator begin(), end();        // 迭代器
```

基类自动提供 deque 操作的默认实现（基于上述基本操作）：

```cpp
void push_front(int x);   // insert(0, x)
void push_back(int x);    // insert(size(), x)
void pop_front();         // remove(0)
void pop_back();          // remove(size()-1)
int front();              // query(0)
int back();               // query(size()-1)
```

详见 `src/deque.hpp`.

当然, 你仍然可以在实现的数据结构中, 重新实现 push_front/push_back 等 deque 接口, 以避免可能的性能问题.

---

## Vector

### STD Vector

作为示例,我们在 `src/std_vector.hpp` 中实现了对 cpp 标准库 vector 的包装, 该实现将作为基线用于校验实现.

### 扩缩容倍数

我们在课上学习了 Vector 的扩缩策略, 现在我们需要在 `src/vector.hpp` 自己实现任意倍数的扩容策略.

对于模板类

```cpp
template<int GrowthNum = 2, int GrowthDen = 1>
class Vector : public DequeInterface<Vector<GrowthNum, GrowthDen>>;
```

其扩容系数为 `R = GrowthNum/GrowthDen`.
例如 `Vector<3,2>` 意味着该 Vector 使用 `R=3/2=1.5` 的扩缩倍数.
默认为 2.

你需要实现 `grow` 和 `shrink` 方法来实现扩缩容量 (**C1**),
当插入导致元素数量超出当前容量`capacity`时, 扩容到 `capacity * R`,
当删除导致元素数量小于等于 `capacity / (R*R)` 时, 缩容到 `capacity / R` 附近. 
测试不同扩缩系数的性能, 是否有理论上差距的那么大 (**T1**)

---

## Linked List

本实验提供三种链表实现，用于对比不同因素对性能的影响.

在 `src/linked_list.hpp` 中, 我们提供了 3 种 Linked List 实现,
观察不同 Linked List 的数据组织方法, 有什么不同?
仅补全不同 Linked List 的 insert 和 remove 方法 (**C2**),
测试不同实现的性能, 并解释对应的结果 (**T2**).

理论上, Linked List 在开始与末尾操作均为 O(1), 目前的实现是这样吗?
尝试优化并重新测试. (**T3**)


### XOR Linked List

回看 Linked List 的实现, 存储一个 int 数值(32bit)需要额外的两个 pointer(在目前主流机器上为 64bit).
内存消耗较大. 
[XOR Linked List](https://en.wikipedia.org/wiki/XOR_linked_list), 
利用 `xor_ptr = prev ^ next`，节省一个指针.
在 `src/xor_list.hpp` 中实现 XOR Linked List 的插入和删除. (**C3**).

---

## 块状链表

[Unrolled Linked List](https://en.wikipedia.org/wiki/Unrolled_linked_list)，每个节点存储至多 B 个元素（chunk），兼顾链表和数组的优点。

```
[chunk0: data[0..B-1]] <-> [chunk1: data[0..B-1]] <-> [chunk2: ...]
```

块状链表可以视为 Chunk 为单位组成的链表.
在操作时定位到对应 chunk 并进行 vector 操作. 
当一个 chunk 的元素数量过多(大于B)时,分裂为两个 chunk.
在删除时,尝试与后一个Chunk合并(仍不超过阈值B).

结合 `vector` 和 `linked_list`, 在 `src/unroll_linked_list.hpp` 中补全块状链表的实现 (**C4**).
在各种任务中测试块状链表的性能, 并简单探讨为什么有这种性能表现. (**T4**)

---

## 数据生成与测试

`main.cpp` 可以接收生成的数据测试性能, 但是不会判断数据是否合法(例如对空序列删除)

框架利用 python 实现了简单的数据生成器 (gen.py), 当然, 你可以实现自己的数据生成器.

### 数据格式

```
n              # n 次操作
I idx val      # insert(idx, val)
R idx          # remove(idx)
Q idx          # query(idx)
F val          # push_front(val)
B val          # push_back(val)
f              # pop_front()
b              # pop_back()
```

### 测试场景

框架 (gen.py) 提供了几种常见情形的测试数据, 你可以在测试任务中从中选取典型的 workload.

请注意, 数据均为随机生成, 可能无法得到显著结论. 你可以自行构造数据, 需要给出构造的**动机**和**方法**.


| 文件 | 操作 | 测试目的 |
|-----|-----|---------|
| `append.txt` | 纯 push_back | Vector 扩容性能、链表 append 开销 |
| `stack.txt` | push_back / pop_back | 栈操作 |
| `queue.txt` | push_back / pop_front | 队列操作 |
| `deque.txt` | 双端混合操作 | 综合 deque 性能 |
| `random_insert.txt` | 随机位置插入 | 随机插入性能 |
| `query.txt` | 构建后大量随机查询 | 随机访问性能 |
| `mixed.txt` | 混合操作 | 综合场景 |

遍历性能通过 `--traverse` 参数单独测试。

### 运行方式

见 [README.md](./README.md)

---

## 思考题 (E)

- **E1**. 在仅有 `push_back` 的情况下, Vector 不同的扩缩倍数，理论的均摊复杂度是什么？给出推导。
- **E2**. 在 gcc, msvc 的标准库中, Vector 是否使用 2 倍系数, 扩缩容策略是否与我们实现的一致.
- **E3**. List 理论上有着更好的插入速度，在测试中是否在一些项目中相较于 Vector 有显著优势？如果没有又是为什么？
- **E4**. 对于 XOR List，如果给定**任意**一个节点，是否可以从此开始向前/后遍历？
- **E5**. 目前 ArrayLinkedList 的数据**内存布局**是否合适？如果不合适，尝试给出一种修改方案，并解释为什么可能有更好的性能。
- **E6**. Python 中的 deque 使用的数据结构是什么？询问 AI 或查找源代码了解，结合你的测试结果解释为什么要这样选择。
- **E7**. 在很多场景中(例如游戏引擎保存所有实体), 我们不需要有序的 list, 只需要能够插入/删除/遍历所有元素. 检查 [godot local vector](https://github.com/godotengine/godot/blob/2327a823578a30f09068f97272598521896d5633/core/templates/local_vector.h) 的 `remove_at_unordered` 以及 Rust 标准库的 [swap_remove](https://doc.rust-lang.org/std/vec/struct.Vec.html#method.swap_remove) 简述原理与预期性能.
- **E8**. 尝试修改 `main.cpp` 中的 DataType 为 `BigData` 并重新测试. 之前的性能差距还存在吗, 是否显著?
- **E9**. 在实际项目中, 我们应该如何选择数据结构 (开放问题, 不计分)

### 提交要求

提交一个 zip 压缩包, 内含有你修改后的最终项目代码. (请**不要**上传编译产物/测试数据文件)
并加上 `report.md` 作为你的报告. 需要包含
- 对代码补全任务**C1-4** 的简要思路描述.
- 测试任务 (**T1-4**) 的结论以及用于作证的测试结果, 如果进行数据构造, 请说简要说明构造的**动机**和**方法**.
- 对思考题 (**E1-9**) 的回答.

描述需要准确易于理解, 避免粘贴大量代码.

