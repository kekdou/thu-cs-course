#pragma once
#include "deque.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>

// ============================================================================
// Vector - 动态数组，支持配置扩容因子
// 
// 模板参数：
//   GrowthNum/GrowthDen: 扩容因子 R = GrowthNum/GrowthDen (如 2/1=2倍, 3/2=1.5倍)
// ============================================================================
template<class T, int GrowthNum = 2, int GrowthDen = 1>
class Vector : public DequeInterface<Vector<T, GrowthNum, GrowthDen>, T> {
    T* data_;
    std::size_t size_;
    std::size_t capacity_;
    
    static constexpr std::size_t INITIAL_CAPACITY = 4;
    
    void grow() {
        // TODO: 实现扩容逻辑
        // 当 size >= capacity 时调用，需要增大 capacity
        std::size_t new_capacity = (capacity_ == 0) ? INITIAL_CAPACITY : capacity_ * GrowthNum / GrowthDen;
        if (new_capacity <= capacity_) {
            new_capacity = capacity_ + 1;
        }
        reserve(new_capacity);
    }
    
    void shrink() {
        // TODO: 实现缩容逻辑
        // 当 size < capacity/(R*R)，将 capacity 缩小
        std::size_t threshold = capacity_ * GrowthDen * GrowthDen / GrowthNum / GrowthNum;
        if (size_ > threshold) {
            return;
        }
        std::size_t new_capacity = capacity_ * GrowthDen / GrowthNum;
        if (new_capacity < INITIAL_CAPACITY) {
            new_capacity = INITIAL_CAPACITY;
        }
        if (new_capacity <= size_) {
            new_capacity = size_ + 1;
        }
        T* old_data = data_;
        data_ = new T[new_capacity];
        std::memcpy(data_, old_data, size_ * sizeof(T));
        delete[] old_data;
        capacity_ = new_capacity;
    }

public:
    Vector() : data_(nullptr), size_(0), capacity_(0) {
        reserve(INITIAL_CAPACITY);
    }
    
    ~Vector() {
        delete[] data_;
    }
    
    Vector(const Vector&) = delete;
    Vector& operator=(const Vector&) = delete;
    
    Vector(Vector&& other) noexcept 
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = other.capacity_ = 0;
    }
    
    void reserve(std::size_t new_cap) {
        if (new_cap <= capacity_) return;
        T* new_data = new T[new_cap];
        if (data_) {
            std::memcpy(new_data, data_, size_ * sizeof(T));
            delete[] data_;
        }
        data_ = new_data;
        capacity_ = new_cap;
    }
    
    void insert(int index, T val) {
        if (size_ >= capacity_) {
            grow();
        }
        // 后移元素
        std::memmove(data_ + index + 1, data_ + index, 
                     (size_ - index) * sizeof(T));
        data_[index] = val;
        ++size_;
    }
    
    void remove(int index) {
        // 前移元素
        std::memmove(data_ + index, data_ + index + 1,
                     (size_ - index - 1) * sizeof(T));
        --size_;
        shrink();
    }
    
    T query(int index) const {
        return data_[index];
    }
    
    T& operator[](int index) {
        return data_[index];
    }
    
    const T& operator[](int index) const {
        return data_[index];
    }
    
    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    
    // 迭代器
    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }
};

// 预定义几种常用配置
template <class T>
using Vector2x = Vector<T, 2, 1>;    // 2倍扩容
template <class T>
using Vector15x = Vector<T, 3, 2>;   // 1.5倍扩容
template <class T>
using Vector3x = Vector<T, 3, 1>;
template <class T>
using Vector4x = Vector<T, 4, 1>;
template <class T>
using Vector8x = Vector<T, 8, 1>;
template <class T>
using Vector16x = Vector<T, 16, 1>;
