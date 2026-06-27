// ============================================================================
// StdVector - std::vector 的包装，用于性能对比和正确性验证
// ============================================================================
#include "deque.hpp"
#include <vector>

template <class T>
class StdVector : public DequeInterface<StdVector<T>, T> {
    std::vector<T> vec_;

public:
    void insert(int index, T val) {
        vec_.insert(vec_.begin() + index, val);
    }
    
    void remove(int index) {
        vec_.erase(vec_.begin() + index);
    }
    
    T query(int index) const {
        return vec_[index];
    }
    
    // 覆盖默认实现，直接用 std::vector 的方法
    void push_back(T x) { vec_.push_back(x); }
    void pop_back() { vec_.pop_back(); }
    
    std::size_t size() const { return vec_.size(); }
    
    auto begin() { return vec_.begin(); }
    auto end() { return vec_.end(); }
    auto begin() const { return vec_.begin(); }
    auto end() const { return vec_.end(); }
};
