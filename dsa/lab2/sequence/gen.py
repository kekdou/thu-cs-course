#!/usr/bin/env python3
"""
数据生成器 - 生成不同场景的测试数据

数据格式：
  n              # n 次操作
  I idx val      # insert(idx, val)
  R idx          # remove(idx)
  Q idx          # query(idx)
  F val          # push_front(val)
  B val          # push_back(val)
  f              # pop_front()
  b              # pop_back()
"""

import random
import argparse
from pathlib import Path


def gen_append(n: int) -> str:
    """纯 push_back 操作"""
    lines = [str(n)]
    for i in range(n):
        lines.append(f"B {i}")
    return "\n".join(lines)

def gen_push_front(n: int) -> str:
    lines = [str(n)]
    for i in range(n):
        lines.append(f"F {i}")
    return "\n".join(lines)

def gen_pop_back(n: int) -> str:
    lines = [str(n)]
    for i in range(n):
        lines.append(f"b")
    return "\n".join(lines)

def gen_pop_front(n: int) -> str:
    lines = [str(n)]
    for i in range(n):
        lines.append(f"f")
    return "\n".join(lines)

def gen_stack(n: int) -> str:
    """栈操作：push_back / pop_back"""
    lines = [str(n)]
    size = 0
    for _ in range(n):
        if size == 0 or random.random() < 0.6:
            lines.append(f"B {random.randint(0, 10**6)}")
            size += 1
        else:
            lines.append("b")
            size -= 1
    return "\n".join(lines)


def gen_queue(n: int) -> str:
    """队列操作：push_back / pop_front"""
    lines = [str(n)]
    size = 0
    for _ in range(n):
        if size == 0 or random.random() < 0.6:
            lines.append(f"B {random.randint(0, 10**6)}")
            size += 1
        else:
            lines.append("f")
            size -= 1
    return "\n".join(lines)


def gen_deque(n: int) -> str:
    """双端队列操作：push/pop front/back"""
    lines = [str(n)]
    size = 0
    for _ in range(n):
        if size == 0:
            op = random.choice(["F", "B"])
        else:
            op = random.choice(["F", "B", "f", "b"])

        if op == "F":
            lines.append(f"F {random.randint(0, 10**6)}")
            size += 1
        elif op == "B":
            lines.append(f"B {random.randint(0, 10**6)}")
            size += 1
        elif op == "f":
            lines.append("f")
            size -= 1
        else:
            lines.append("b")
            size -= 1
    return "\n".join(lines)


def gen_random_insert(n: int) -> str:
    """随机位置插入"""
    lines = [str(n)]
    size = 0
    for _ in range(n):
        idx = random.randint(0, size)
        lines.append(f"I {idx} {random.randint(0, 10**6)}")
        size += 1
    return "\n".join(lines)

def gen_random_remove(n: int) -> str:
    lines = [str(n)]
    size = n - 1
    for _ in range(n):
        idx = random.randint(0, size)
        lines.append(f"R {idx}")
        size -= 1
    return "\n".join(lines)

def gen_query(n: int) -> str:
    """先构建，再大量查询"""
    ops = n
    lines = [str(ops)]
    # 查询
    for _ in range(n):
        idx = random.randint(0, n - 1)
        lines.append(f"Q {idx}")
    return "\n".join(lines)


def gen_mixed(n: int) -> str:
    """混合操作"""
    lines = [str(n)]
    size = 0
    for _ in range(n):
        if size == 0:
            op = random.choice(["F", "B", "I"])
        elif size < 10:
            op = random.choice(["F", "B", "I", "Q"])
        else:
            op = random.choice(["F", "B", "f", "b", "I", "R", "Q"])

        if op == "F":
            lines.append(f"F {random.randint(0, 10**6)}")
            size += 1
        elif op == "B":
            lines.append(f"B {random.randint(0, 10**6)}")
            size += 1
        elif op == "f":
            lines.append("f")
            size -= 1
        elif op == "b":
            lines.append("b")
            size -= 1
        elif op == "I":
            idx = random.randint(0, size)
            lines.append(f"I {idx} {random.randint(0, 10**6)}")
            size += 1
        elif op == "R":
            idx = random.randint(0, size - 1)
            lines.append(f"R {idx}")
            size -= 1
        else:  # Q
            idx = random.randint(0, size - 1)
            lines.append(f"Q {idx}")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description="生成测试数据")
    parser.add_argument(
        "--type",
        choices=[
            "append",
            "stack",
            "queue",
            "deque",
            "random_insert",
            "query",
            "mixed",
            "all",
            "pop_back", 
            "random_remove", 
            "pop_front", 
            "push_front"
        ],
        default="all",
        help="数据类型",
    )
    parser.add_argument("-n", type=int, default=100000, help="操作数量")
    parser.add_argument("--build", type=int, default=10000, help="query 测试的构建数量")
    parser.add_argument("--seed", type=int, default=42, help="随机种子")
    parser.add_argument("-o", "--output", type=str, default="data", help="输出目录")
    args = parser.parse_args()

    random.seed(args.seed)
    output_dir = Path(args.output)
    output_dir.mkdir(exist_ok=True)

    generators = {
        "append": lambda: gen_append(args.n),
        "stack": lambda: gen_stack(args.n),
        "queue": lambda: gen_queue(args.n),
        "deque": lambda: gen_deque(args.n),
        "random_insert": lambda: gen_random_insert(args.n),
        "query": lambda: gen_query(args.n),
        "mixed": lambda: gen_mixed(args.n),
        "pop_back": lambda: gen_pop_back(args.n), 
        "random_remove": lambda: gen_random_remove(args.n), 
        "pop_front": lambda: gen_pop_front(args.n), 
        "push_front": lambda: gen_push_front(args.n)
    }

    if args.type == "all":
        for name, gen in generators.items():
            random.seed(args.seed)  # 每个测试用相同的种子
            data = gen()
            (output_dir / f"{name}.txt").write_text(data)
            print(f"Generated {name}.txt")
    else:
        data = generators[args.type]()
        (output_dir / f"{args.type}.txt").write_text(data)
        print(f"Generated {args.type}.txt")


if __name__ == "__main__":
    main()
