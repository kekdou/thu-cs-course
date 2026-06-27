CXX ?= g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

# 数据规模变量，可被命令行覆盖
N ?= 10000
N_SMALL ?= 10000
N_LARGE ?= 500000

.PHONY: all clean run gen test bench check

all: benchmark

benchmark: main.cpp src/*.hpp
	$(CXX) $(CXXFLAGS) -o $@ main.cpp

# 生成测试数据
gen:
	python3 gen.py --type all -n $(N)

# 运行所有测试
run: benchmark gen
	./benchmark --all

# 正确性检查
check: benchmark gen
	./benchmark --check --all

# 快速测试（小规模）
test: benchmark
	python3 gen.py --type all -n $(N_SMALL)
	./benchmark --all

# 完整测试（大规模）
bench: benchmark
	python3 gen.py --type all -n $(N_LARGE)
	./benchmark --all

clean:
	rm -f benchmark
	rm -f data/*.txt
