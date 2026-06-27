#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

#include "src/std_vector.hpp"
#include "src/vector.hpp"
#include "src/linked_list.hpp"
#include "src/unroll_linked_list.hpp"
#include "src/xor_list.hpp"

// ============================================================================
// 操作定义与读取
// ============================================================================
struct Operation {
    char op;
    int idx;
    int val;
};

struct BigData {
    int value[32];
    BigData(int raw_value = 0) {
      value[0] = raw_value;
      value[31] = raw_value + 1;
    }
    operator int() const {
      return value[0];
    }
};

using DataType = BigData;

std::vector<Operation> read_operations(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        exit(1);
    }
    
    int n;
    fin >> n;
    std::vector<Operation> ops(n);
    
    for (int i = 0; i < n; ++i) {
        fin >> ops[i].op;
        ops[i].idx = 0;
        int raw_value = 0;
        switch (ops[i].op) {
            case 'I': fin >> ops[i].idx >> raw_value; break;
            case 'R': case 'Q': fin >> ops[i].idx; break;
            case 'F': case 'B': fin >> raw_value; break;
        }
        ops[i].val = DataType(raw_value);
    }
    return ops;
}

std::string extract_test_name(const std::string& filename) {
    auto p1 = filename.find_last_of('/');
    auto start = (p1 == std::string::npos) ? 0 : p1 + 1;
    auto p2 = filename.find_last_of('.');
    return filename.substr(start, p2 - start);
}

// ============================================================================
// 执行操作
// ============================================================================
template<typename Container>
void execute_ops(Container& c, const std::vector<Operation>& ops, std::vector<int>* results = nullptr) {
    for (const auto& op : ops) {
        switch (op.op) {
            case 'I': c.insert(op.idx, DataType(op.val)); break;
            case 'R': c.remove(op.idx); break;
            case 'Q': 
                if (results) results->push_back(int(c.query(op.idx)));
                else (void)c.query(op.idx);
                break;
            case 'F': c.push_front(op.val); break;
            case 'B': c.push_back(op.val); break;
            case 'f': c.pop_front(); break;
            case 'b': c.pop_back(); break;
        }
    }
}

// ============================================================================
// Check 模式
// ============================================================================
enum class Status { OK, WRONG, SKIP };

template<typename Container>
Status check_container(const std::vector<Operation>& ops, 
                       const std::vector<int>& expected_queries,
                       const std::vector<int>& expected_elements) {
    try {
        Container c;
        std::vector<int> query_results;
        execute_ops(c, ops, &query_results);
        
        if (query_results != expected_queries) return Status::WRONG;
        
        if (c.size() != expected_elements.size()) return Status::WRONG;
        
        std::vector<int> actual_elements;
        for (auto it = c.begin(); it != c.end(); ++it) {
            actual_elements.push_back(int(*it));
        }
        if (actual_elements != expected_elements) return Status::WRONG;
        
        return Status::OK;
    } catch (...) {
        return Status::SKIP;
    }
}

void run_check(const std::string& filename) {
    auto ops = read_operations(filename);
    
    StdVector<DataType> baseline;
    std::vector<int> expected_queries;
    execute_ops(baseline, ops, &expected_queries);
    
    std::vector<int> expected_elements;
    for (auto it = baseline.begin(); it != baseline.end(); ++it) {
        expected_elements.push_back(int(*it));
    }
    
    std::vector<std::pair<std::string, Status>> results = {
        {"std::vector",       Status::OK},
        {"Vector<2x>",        check_container<Vector2x<DataType>>(ops, expected_queries, expected_elements)},
        {"Vector<1.5x>",      check_container<Vector15x<DataType>>(ops, expected_queries, expected_elements)},
        {"PtrLinkedList",     check_container<PtrLinkedList<DataType>>(ops, expected_queries, expected_elements)},
        {"PoolPtrLinkedList", check_container<PoolPtrLinkedList<DataType>>(ops, expected_queries, expected_elements)},
        {"ArrayLinkedList",   check_container<ArrayLinkedList<DataType>>(ops, expected_queries, expected_elements)},
        {"UnrollLinkedList<64>",   check_container<UnrollLinkedList64<DataType>>(ops, expected_queries, expected_elements)},
        {"UnrollLinkedList<256>",  check_container<UnrollLinkedList256<DataType>>(ops, expected_queries, expected_elements)},
        {"XorLinkedList",     check_container<XorLinkedList<DataType>>(ops, expected_queries, expected_elements)},
    };
    
    std::cout << "=== " << extract_test_name(filename) << " (n=" << ops.size() << ", size=" << expected_elements.size() << ") ===" << std::endl;
    for (const auto& [name, status] : results) {
        const char* s = (status == Status::OK) ? "OK" : (status == Status::WRONG) ? "WRONG" : "SKIP";
        std::cout << "  " << std::left << std::setw(22) << name << s << std::endl;
    }
    std::cout << std::endl;
}

// ============================================================================
// Benchmark 模式
// ============================================================================
template<typename Container>
double benchmark_container(const std::vector<Operation>& ops) {
    try {
        Container c;
        auto start = std::chrono::high_resolution_clock::now();
        execute_ops(c, ops);
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    } catch (...) {
        return -1;  // SKIP
    }
}

template<typename Container>
double benchmark_traverse(std::size_t n, std::size_t m) {
    try {
        Container c;
        for (std::size_t i = 0; i < n; ++i) c.push_back(i);
        volatile long long sum = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (std::size_t j = 0; j < m; ++j)
            for (auto it = c.begin(); it != c.end(); ++it) sum += *it;
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    } catch (...) {
        return -1;
    }
}

void print_benchmark(const std::string& name, const std::vector<std::pair<std::string, double>>& results, std::size_t ops) {
    std::cout << "=== " << name << " ===" << std::endl;
    std::cout << std::left << std::setw(22) << "Container"
              << std::right << std::setw(10) << "Time(ms)" << std::setw(12) << "ns/op" << std::endl;
    std::cout << std::string(44, '-') << std::endl;
    
    for (const auto& [n, t] : results) {
        std::cout << std::left << std::setw(22) << n;
        if (t < 0) std::cout << std::right << std::setw(10) << "SKIP" << std::endl;
        else std::cout << std::right << std::setw(10) << std::fixed << std::setprecision(2) << t
                       << std::setw(12) << std::setprecision(1) << (t * 1e6 / ops) << std::endl;
    }
    std::cout << std::endl;
}

void run_benchmark(const std::string& filename) {
    auto ops = read_operations(filename);
    std::vector<std::pair<std::string, double>> results = {
        // {"std::vector",       benchmark_container<StdVector<DataType>>(ops)},
        // {"Vector<1.5x>",      benchmark_container<Vector15x<DataType>>(ops)},
        // {"Vector<2x>",        benchmark_container<Vector2x<DataType>>(ops)},
        // {"Vector<4x>",        benchmark_container<Vector4x<DataType>>(ops)},
        // {"Vector<8x>",        benchmark_container<Vector8x<DataType>>(ops)},
        {"PtrLinkedList",     benchmark_container<PtrLinkedList<DataType>>(ops)},
        {"PoolPtrLinkedList", benchmark_container<PoolPtrLinkedList<DataType>>(ops)},
        {"ArrayLinkedList",   benchmark_container<ArrayLinkedList<DataType>>(ops)},
        {"UnrollLinkedList<64>",   benchmark_container<UnrollLinkedList64<DataType>>(ops)},
        {"UnrollLinkedList<256>",  benchmark_container<UnrollLinkedList256<DataType>>(ops)},
        // {"XorLinkedList",     benchmark_container<XorLinkedList<DataType>>(ops)},
    };
    
    print_benchmark(extract_test_name(filename) + " (n=" + std::to_string(ops.size()) + ")", results, ops.size());
}

template<typename Container>
double benchmark_container2(const std::vector<Operation>& ops) {
    try {
        Container c;
        auto ops2 = read_operations("data/append.txt");
        execute_ops(c, ops2);
        auto start = std::chrono::high_resolution_clock::now();
        execute_ops(c, ops);
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    } catch (...) {
        return -1;  // SKIP
    }
}

void run_benchmark2(const std::string& filename) {
    auto ops = read_operations(filename);
    
    std::vector<std::pair<std::string, double>> results = {
        // {"std::vector",       benchmark_container2<StdVector<DataType>>(ops)},
        // {"Vector<1.5x>",      benchmark_container<Vector15x<DataType>>(ops)},
        // {"Vector<2x>",        benchmark_container<Vector2x<DataType>>(ops)},
        // {"Vector<4x>",        benchmark_container<Vector4x<DataType>>(ops)},
        // {"Vector<8x>",        benchmark_container<Vector8x<DataType>>(ops)},
        {"PtrLinkedList",     benchmark_container2<PtrLinkedList<DataType>>(ops)},
        {"PoolPtrLinkedList", benchmark_container2<PoolPtrLinkedList<DataType>>(ops)},
        {"ArrayLinkedList",   benchmark_container2<ArrayLinkedList<DataType>>(ops)},
        {"UnrollLinkedList<64>",   benchmark_container2<UnrollLinkedList64<DataType>>(ops)},
        {"UnrollLinkedList<256>",  benchmark_container2<UnrollLinkedList256<DataType>>(ops)},
        // {"XorLinkedList",     benchmark_container2<XorLinkedList<DataType>>(ops)},
    };
    
    print_benchmark(extract_test_name(filename) + " (n=" + std::to_string(ops.size()) + ")", results, ops.size());
}

void run_traverse(std::size_t n, std::size_t m) {
    std::vector<std::pair<std::string, double>> results = {
        // {"std::vector",       benchmark_traverse<StdVector<DataType>>(n, m)},
        // {"Vector<2x>",        benchmark_traverse<Vector2x<DataType>>(n, m)},
        // {"PtrLinkedList",     benchmark_traverse<PtrLinkedList<DataType>>(n, m)},
        // {"PoolPtrLinkedList", benchmark_traverse<PoolPtrLinkedList<DataType>>(n, m)},
        // {"ArrayLinkedList",   benchmark_traverse<ArrayLinkedList<DataType>>(n, m)},
        {"UnrollLinkedList<64>",   benchmark_traverse<UnrollLinkedList64<DataType>>(n, m)},
        {"UnrollLinkedList<256>",  benchmark_traverse<UnrollLinkedList256<DataType>>(n, m)},
        // {"XorLinkedList",     benchmark_traverse<XorLinkedList<DataType>>(n, m)},
    };
    print_benchmark("traverse (n=" + std::to_string(n) + ", m=" + std::to_string(m) + ")", results, n * m);
}

// ============================================================================
// Main
// ============================================================================
const std::vector<std::string> ALL_FILES = {
    "data/append.txt", "data/stack.txt", "data/queue.txt",
    "data/deque.txt", "data/random_insert.txt", "data/query.txt", "data/mixed.txt", 
    "data/pop_back.txt", "data/random_remove.txt", "data/pop_front.txt", 
    "data/push_front.txt"
};

void print_usage(const char* prog) {
    std::cout << "Usage:\n"
              << "  " << prog << " [--check] <file.txt> ...   Run benchmark or check\n"
              << "  " << prog << " [--check] --all            Run all tests\n"
              << "  " << prog << " --traverse -n N -m M       Traverse benchmark\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) { print_usage(argv[0]); return 1; }
    
    bool check_mode = false;
    std::vector<std::string> files;
    std::size_t n = 10000, m = 1000;
    bool do_traverse = false, do_all = false;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--check") check_mode = true;
        else if (arg == "--all") do_all = true;
        else if (arg == "--traverse") do_traverse = true;
        else if (arg == "-n" && i+1 < argc) n = std::stoul(argv[++i]);
        else if (arg == "-m" && i+1 < argc) m = std::stoul(argv[++i]);
        else if (arg == "-h" || arg == "--help") { print_usage(argv[0]); return 0; }
        else files.push_back(arg);
    }
    
    std::cout << "========================================\n"
              << "  Data Structure " << (check_mode ? "Check" : "Benchmark") << "\n"
              << "========================================\n\n";
    
    if (do_all) files = ALL_FILES;
    
    for (const auto& f : files) {
        std::ifstream test(f);
        if (test.good()) {
            if (check_mode) run_check(f);
            else {
                if (extract_test_name(f) == "pop_back" || extract_test_name(f) == "pop_front" || extract_test_name(f) == "random_remove" || "query") {
                    run_benchmark2(f);
                } else {
                    run_benchmark(f);
                }
            }
        }
    }
    
    // if (do_traverse && !check_mode) run_traverse(n, m);
    // if (do_all && !check_mode) run_traverse(n, m);
    
    return 0;
}
