#pragma once
#include "deque.hpp"
#include <cstddef>
#include <iterator>
#include <stdexcept>

// ============================================================================
// 1. PtrLinkedList
// ============================================================================
template <class T>
class PtrLinkedList : public DequeInterface<PtrLinkedList<T>, T> {
    struct Node {
        T val;
        Node* prev;
        Node* next;
        Node(T v = T(), Node* p = nullptr, Node* n = nullptr) 
            : val(v), prev(p), next(n) {}
    };
    
    Node* head_;    // dummy head
    Node* tail_;    // dummy tail
    std::size_t size_;
    
    // 获取第 index 个节点
    Node* get_node(int index) const {
        Node* cur = head_->next;
        for (int i = 0; i < index; ++i) {
            cur = cur->next;
        }
        return cur;
    }

public:
    PtrLinkedList() : size_(0) {
        head_ = new Node();
        tail_ = new Node();
        head_->next = tail_;
        tail_->prev = head_;
    }
    
    ~PtrLinkedList() {
        Node* cur = head_;
        while (cur) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
    }
    
    // 禁止拷贝
    PtrLinkedList(const PtrLinkedList&) = delete;
    PtrLinkedList& operator=(const PtrLinkedList&) = delete;
    
    // 移动构造
    PtrLinkedList(PtrLinkedList&& other) noexcept 
        : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = other.tail_ = nullptr;
        other.size_ = 0;
    }
    
    void insert(int index, T val) {
        Node* next_node = (std::size_t(index) == size_) ? tail_ : get_node(index);
        Node* prev_node = next_node->prev;
        Node* new_node = new Node(val, prev_node, next_node);
        prev_node->next = new_node;
        next_node->prev = new_node;
        ++size_;
    }
    
    void remove(int index) {
        Node* node = get_node(index);
        node->prev->next = node->next;
        node->next->prev = node->prev;
        delete node;
        --size_;
    }
    
    void push_front(T val) {
        Node* next_node = head_->next;
        Node* new_node = new Node(val, head_, next_node);
        head_->next = new_node;
        next_node->prev = new_node;
        size_++;
    }

    void push_back(T val) {
        Node* prev_node = tail_->prev;
        Node* new_node = new Node(val, prev_node, tail_);
        tail_->prev = new_node;
        prev_node->next = new_node;
        size_++;
    }

    void pop_front() {
        Node* node = head_->next;
        if (node == tail_) {
            return;
        }
        head_->next = node->next;
        node->next->prev = head_;
        delete node;
        size_--;
    }

    void pop_back() {
        Node* node = tail_->prev;
        if (node == head_) {
            return;
        }
        tail_->prev = node->prev;
        node->prev->next = tail_;
        delete node;
        size_--;
    }

    T query(int index) const {
        return get_node(index)->val;
    }
    
    std::size_t size() const { return size_; }
    
    // 迭代器
    class Iterator {
        Node* cur_;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        
        Iterator(Node* n) : cur_(n) {}
        T& operator*() { return cur_->val; }
        Iterator& operator++() { cur_ = cur_->next; return *this; }
        Iterator& operator--() { cur_ = cur_->prev; return *this; }
        bool operator==(const Iterator& o) const { return cur_ == o.cur_; }
        bool operator!=(const Iterator& o) const { return cur_ != o.cur_; }
    };
    
    Iterator begin() { return Iterator(head_->next); }
    Iterator end() { return Iterator(tail_); }
};


// ============================================================================
// 2. PoolPtrLinkedList
// ============================================================================
template <class T>
class PoolPtrLinkedList : public DequeInterface<PoolPtrLinkedList<T>, T> {
    struct Node {
        T val;
        Node* prev;
        Node* next;
    };
    
    // 内存池
    static constexpr std::size_t POOL_SIZE = 1 << 20;  // 1M 节点
    Node* pool_;
    Node* free_head_;   // 空闲链表头
    
    Node* head_;
    Node* tail_;
    std::size_t size_;
    
    Node* alloc_node() {
        Node* node = free_head_;
        free_head_ = free_head_->next;
        return node;
    }
    
    void free_node(Node* node) {
        node->next = free_head_;
        free_head_ = node;
    }
    
    Node* get_node(int index) const {
        Node* cur = head_->next;
        for (int i = 0; i < index; ++i) {
            cur = cur->next;
        }
        return cur;
    }

public:
    PoolPtrLinkedList() : size_(0) {
        // 分配内存池
        pool_ = new Node[POOL_SIZE];
        // 初始化空闲链表
        free_head_ = &pool_[0];
        for (std::size_t i = 0; i < POOL_SIZE - 1; ++i) {
            pool_[i].next = &pool_[i + 1];
        }
        pool_[POOL_SIZE - 1].next = nullptr;
        
        // 分配 dummy 节点
        head_ = alloc_node();
        tail_ = alloc_node();
        head_->prev = nullptr;
        head_->next = tail_;
        tail_->prev = head_;
        tail_->next = nullptr;
    }
    
    ~PoolPtrLinkedList() {
        delete[] pool_;
    }
    
    PoolPtrLinkedList(const PoolPtrLinkedList&) = delete;
    PoolPtrLinkedList& operator=(const PoolPtrLinkedList&) = delete;
    
    void insert(int index, T val) {
        // TODO: 在位置 index 插入值 val
        // 参考 PtrLinkedList::insert 的实现
        Node* new_node = alloc_node();
        new_node->val = val;
        Node* next_node = get_node(index);
        Node* prev_node = next_node->prev;
        new_node->next = next_node;
        next_node->prev = new_node;
        prev_node->next = new_node;
        new_node->prev = prev_node;
        size_++;
    }
    
    void remove(int index) {
        // TODO: 删除位置 index 的元素
        // 参考 PtrLinkedList::remove 的实现
        Node* node = get_node(index);
        Node* prev_node = node->prev;
        Node* next_node = node->next;
        prev_node->next = next_node;
        next_node->prev = prev_node;
        free_node(node);
        size_--;
    }
    
    void push_front(T val) {
        Node* new_node = alloc_node();
        new_node->val = val;
        new_node->next = head_->next;
        new_node->prev = head_;
        Node* next_node = head_->next;
        next_node->prev = new_node;
        head_->next = new_node;
        size_++;
    }

    void push_back(T val) {
        Node* new_node = alloc_node();
        new_node->val = val;
        new_node->prev = tail_->prev;
        new_node->next = tail_;
        Node* prev_node = tail_->prev;
        prev_node->next = new_node;
        tail_->prev = new_node;
        size_++;
    }

    void pop_front() {
        Node* node = head_->next;
        if (node == tail_) {
            return;
        }
        head_->next = node->next;
        node->next->prev = head_;
        free_node(node);
        size_--;
    }

    void pop_back() {
        Node* node = tail_->prev;
        if (node == head_) {
            return;
        }
        tail_->prev = node->prev;
        node->prev->next = tail_;
        free_node(node);
        size_--;
    }
    
    T query(int index) const {
        return get_node(index)->val;
    }
    
    std::size_t size() const { return size_; }
    
    // 迭代器
    class Iterator {
        Node* cur_;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        
        Iterator(Node* n) : cur_(n) {}
        T& operator*() { return cur_->val; }
        Iterator& operator++() { cur_ = cur_->next; return *this; }
        Iterator& operator--() { cur_ = cur_->prev; return *this; }
        bool operator==(const Iterator& o) const { return cur_ == o.cur_; }
        bool operator!=(const Iterator& o) const { return cur_ != o.cur_; }
    };
    
    Iterator begin() { return Iterator(head_->next); }
    Iterator end() { return Iterator(tail_); }
};


// ============================================================================
// 3. ArrayLinkedList
// ============================================================================
template <class T>
class ArrayLinkedList : public DequeInterface<ArrayLinkedList<T>, T> {
    static constexpr std::size_t POOL_SIZE = 1 << 20;
    
    T* val_;
    int* prev_;
    int* next_;
    
    int free_head_;     // 空闲链表头索引
    int head_;          // dummy head 索引
    int tail_;          // dummy tail 索引
    std::size_t size_;
    
    int alloc_node() {
        int idx = free_head_;
        free_head_ = next_[free_head_];
        return idx;
    }
    
    void free_node(int idx) {
        next_[idx] = free_head_;
        free_head_ = idx;
    }
    
    int get_node_idx(int index) const {
        int cur = next_[head_];
        for (int i = 0; i < index; ++i) {
            cur = next_[cur];
        }
        return cur;
    }

public:
    ArrayLinkedList() : size_(0) {
        val_ = new T[POOL_SIZE];
        prev_ = new int[POOL_SIZE];
        next_ = new int[POOL_SIZE];
        
        // 初始化空闲链表
        free_head_ = 0;
        for (std::size_t i = 0; i < POOL_SIZE - 1; ++i) {
            next_[i] = i + 1;
        }
        next_[POOL_SIZE - 1] = -1;
        
        // 分配 dummy 节点
        head_ = alloc_node();
        tail_ = alloc_node();
        prev_[head_] = -1;
        next_[head_] = tail_;
        prev_[tail_] = head_;
        next_[tail_] = -1;
    }
    
    ~ArrayLinkedList() {
        delete[] val_;
        delete[] prev_;
        delete[] next_;
    }
    
    ArrayLinkedList(const ArrayLinkedList&) = delete;
    ArrayLinkedList& operator=(const ArrayLinkedList&) = delete;
    
    void insert(int index, T val) {
        // TODO: 在位置 index 插入值 val
        // 使用 SoA 布局：val_[], prev_[], next_[]
        int idx = alloc_node();
        val_[idx] = val;
        int next_idx = get_node_idx(index);
        int prev_idx = prev_[next_idx];
        next_[prev_idx] = idx;
        prev_[idx] = prev_idx;
        next_[idx] = next_idx;
        prev_[next_idx] = idx;
        size_++;
    }
    
    void remove(int index) {
        // TODO: 删除位置 index 的元素
        int idx = get_node_idx(index);
        int prev_idx = prev_[idx];
        int next_idx = next_[idx];
        next_[prev_idx]= next_idx;
        prev_[next_idx] = prev_idx;
        free_node(idx);
        size_--;
    }
    
    void push_front(T val) {
        int idx = alloc_node();
        val_[idx] = val;
        next_[idx] = next_[head_];
        prev_[idx] = head_;
        prev_[next_[head_]] = idx;
        next_[head_] = idx;
        size_++;
    }

    void push_back(T val) {
        int idx = alloc_node();
        val_[idx] = val;
        next_[idx] = tail_;
        prev_[idx] = prev_[tail_];
        next_[prev_[tail_]] = idx;
        prev_[tail_] = idx;
        size_++;
    }

    void pop_front() {
        int idx = next_[head_];
        if (idx == tail_) {
            return;
        }
        next_[head_] = next_[idx];
        prev_[next_[idx]] = head_;
        free_node(idx);
        size_--;
    }

    void pop_back() {
        int idx = prev_[tail_];
        if (idx == head_) {
            return;
        }
        prev_[tail_] = prev_[idx];
        next_[prev_[idx]] = tail_;
        free_node(idx);
        size_--;
    }
    
    T query(int index) const {
        return val_[get_node_idx(index)];
    }
    
    std::size_t size() const { return size_; }
    
    // 迭代器
    class Iterator {
        T* val_;
        int* next_;
        int* prev_;
        int cur_;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        
        Iterator(T* val, int* prev, int* next, int idx) 
            : val_(val), next_(next), prev_(prev), cur_(idx) {}
        T& operator*() { return val_[cur_]; }
        Iterator& operator++() { cur_ = next_[cur_]; return *this; }
        Iterator& operator--() { cur_ = prev_[cur_]; return *this; }
        bool operator==(const Iterator& o) const { return cur_ == o.cur_; }
        bool operator!=(const Iterator& o) const { return cur_ != o.cur_; }
    };
    
    Iterator begin() { return Iterator(val_, prev_, next_, next_[head_]); }
    Iterator end() { return Iterator(val_, prev_, next_, tail_); }
};



