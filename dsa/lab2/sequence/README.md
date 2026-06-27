# Lab2: Sequence

实现多种 Vector/Linked List 数据结构，并进行性能对比测试。

## 环境要求

- **编译器**: GCC 或 Clang (支持 C++17)
- **Python**: 3.x (用于生成测试数据)

## 快速开始

```bash
# 编译
make benchmark

# 正确性检查
make check

# 性能测试
make run
```

## 测试数据生成

使用 `gen.py` 生成测试数据：

```bash
# 生成所有测试文件 (默认 n=100000)
python3 gen.py --type all -n 100000

# 生成单个测试文件
python3 gen.py --type append -n 50000

# 可选参数
--type    # append, stack, queue, deque, random_insert, query, mixed, all
-n        # 操作数量 (默认 100000)
--build   # query 测试的构建数量 (默认 10000)
--seed    # 随机种子 (默认 42)
```

## Makefile 用法

```bash
make                    # 编译
make check              # 正确性检查 (n=10000)
make run                # 性能测试 (n=10000)
make test               # 快速测试 (n=10000)
make bench              # 完整测试 (n=500000)

# 自定义数据规模
make check N=5000
make run N=100000
make bench N_LARGE=1000000

# 使用 Clang 编译
make CXX=clang++
```

## 文件结构

```
src/
├── deque.hpp           # CRTP 基类接口
├── std_vector.hpp      # std::vector 包装
├── vector.hpp          # 自定义扩缩容 Vector (TODO)
├── linked_list.hpp     # 三种链表实现 (TODO)
├── xor_list.hpp        # XOR 链表 (TODO)
└── unroll_linked_list.hpp  # 块状链表 (TODO)
```
