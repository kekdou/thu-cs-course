#pragma once
#include "deque.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>

// ============================================================================
// UnrollLinkedList (Unrolled Linked List) - 分块链表
// 
// 每个节点存储 ChunkSize 个元素
// ============================================================================
template<class T, int ChunkSize = 64>
class UnrollLinkedList : public DequeInterface<UnrollLinkedList<T, ChunkSize>, T> {
    struct Chunk {
        T data[ChunkSize];
        int size;           // 当前 chunk 中的元素数量
        Chunk* prev;
        Chunk* next;
        
        Chunk() : size(0), prev(nullptr), next(nullptr) {}
    };
    
    Chunk* head_;           // dummy head
    Chunk* tail_;           // dummy tail
    std::size_t size_;      // 总元素数量
    std::size_t chunk_count_;
    
    // 定位第 index 个元素所在的 chunk 和 chunk 内偏移
    std::pair<Chunk*, int> locate(int index) const {
        Chunk* cur = head_->next;
        int remaining = index;
        while (cur != tail_ && remaining >= cur->size) {
            remaining -= cur->size;
            cur = cur->next;
        }
        return {cur, remaining};
    }
    
    // 在 chunk 的 pos 位置插入元素
    void insert_in_chunk(Chunk* chunk, int pos, T val) {
        // 后移元素
        std::memmove(chunk->data + pos + 1, chunk->data + pos,
                     (chunk->size - pos) * sizeof(T));
        chunk->data[pos] = val;
        chunk->size++;
    }
    
    // 从 chunk 的 pos 位置删除元素
    void remove_from_chunk(Chunk* chunk, int pos) {
        std::memmove(chunk->data + pos, chunk->data + pos + 1,
                     (chunk->size - pos - 1) * sizeof(T));
        chunk->size--;
    }
    
    // 在 chunk 之后插入新 chunk
    Chunk* insert_chunk_after(Chunk* chunk) {
        Chunk* new_chunk = new Chunk();
        new_chunk->prev = chunk;
        new_chunk->next = chunk->next;
        chunk->next->prev = new_chunk;
        chunk->next = new_chunk;
        chunk_count_++;
        return new_chunk;
    }
    
    // 删除 chunk
    void remove_chunk(Chunk* chunk) {
        chunk->prev->next = chunk->next;
        chunk->next->prev = chunk->prev;
        delete chunk;
        chunk_count_--;
    }
    
    // 分裂：当 chunk 满时，分成两半
    void split_chunk(Chunk* chunk) {
        // TODO: 将 chunk 分裂成两个 chunk
        // 1. 在 chunk 之后插入新 chunk
        // 2. 将 chunk 后半部分的数据移动到新 chunk
        Chunk* new_chunk = insert_chunk_after(chunk);
        std::memmove(new_chunk->data, chunk->data + ChunkSize / 2, (ChunkSize - ChunkSize / 2) * sizeof(T));
        chunk->size = ChunkSize / 2;
        new_chunk->size = ChunkSize - ChunkSize / 2;
    }
    
    // 合并：当相邻 chunk 太小时合并
    void try_merge(Chunk* chunk) {
        // TODO: 尝试将 chunk 与后一个 chunk 合并
        // 条件：两个 chunk 的元素总数 <= ChunkSize
        Chunk* next_chunk = chunk->next;
        if (next_chunk->size + chunk->size > ChunkSize) {
            return;
        }
        std::memmove(chunk->data + chunk->size, next_chunk->data, next_chunk->size * sizeof(T));
        chunk->size = chunk->size + next_chunk->size;
        remove_chunk(next_chunk);
    }

public:
    UnrollLinkedList() : size_(0), chunk_count_(0) {
        head_ = new Chunk();
        tail_ = new Chunk();
        head_->next = tail_;
        tail_->prev = head_;
        
        // 初始创建一个空 chunk
        insert_chunk_after(head_);
    }
    
    ~UnrollLinkedList() {
        Chunk* cur = head_;
        while (cur) {
            Chunk* next = cur->next;
            delete cur;
            cur = next;
        }
    }
    
    UnrollLinkedList(const UnrollLinkedList&) = delete;
    UnrollLinkedList& operator=(const UnrollLinkedList&) = delete;
    
    void insert(int index, T val) {
        if (size_ == 0) {
            // 特殊情况：空列表
            Chunk* chunk = head_->next;
            chunk->data[0] = val;
            chunk->size = 1;
            size_++;
            return;
        }
        
        // 定位
        auto [chunk, pos] = locate(index);
        
        // 如果定位到了 tail（index == size_），使用最后一个实际 chunk
        if (chunk == tail_) {
            chunk = tail_->prev;
            pos = chunk->size;
        }
        
        // 检查是否需要分裂
        if (chunk->size >= ChunkSize) {
            split_chunk(chunk);
            // 重新定位
            if (pos >= chunk->size) {
                pos -= chunk->size;
                chunk = chunk->next;
            }
        }
        
        insert_in_chunk(chunk, pos, val);
        size_++;
    }
    
    // 覆盖默认的 push_back，避免遍历整个列表
    void push_back(T val) {
        Chunk* last = tail_->prev;
        
        if (last == head_ || last->size >= ChunkSize) {
            // 需要新建 chunk
            last = insert_chunk_after(last == head_ ? head_ : last);
        }
        
        last->data[last->size++] = val;
        size_++;
    }
    
    // 覆盖默认的 push_front，避免遍历
    void push_front(T val) {
        Chunk* first = head_->next;
        
        if (first == tail_ || first->size >= ChunkSize) {
            // 需要新建 chunk
            first = insert_chunk_after(head_);
        }
        
        // 移动元素腾出位置
        std::memmove(first->data + 1, first->data, first->size * sizeof(T));
        first->data[0] = val;
        first->size++;
        size_++;
    }
    
    // 覆盖默认的 pop_back
    void pop_back() {
        Chunk* last = tail_->prev;
        last->size--;
        size_--;
        
        if (last->size == 0 && chunk_count_ > 1) {
            remove_chunk(last);
        }
    }
    
    // 覆盖默认的 pop_front
    void pop_front() {
        Chunk* first = head_->next;
        std::memmove(first->data, first->data + 1, (first->size - 1) * sizeof(T));
        first->size--;
        size_--;
        
        if (first->size == 0 && chunk_count_ > 1) {
            remove_chunk(first);
        }
    }
    
    void remove(int index) {
        auto [chunk, pos] = locate(index);
        remove_from_chunk(chunk, pos);
        size_--;
        
        // 如果 chunk 变空且不是唯一的 chunk
        if (chunk->size == 0 && chunk_count_ > 1) {
            remove_chunk(chunk);
        } else if (chunk->next != tail_) {
            // 尝试合并
            try_merge(chunk);
        }
    }
    
    T query(int index) const {
        auto [chunk, pos] = locate(index);
        return chunk->data[pos];
    }
    
    std::size_t size() const { return size_; }
    std::size_t chunk_count() const { return chunk_count_; }
    
    // 迭代器
    class Iterator {
        Chunk* chunk_;
        int pos_;
        Chunk* tail_;
        
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        
        Iterator(Chunk* chunk, int pos, Chunk* tail) 
            : chunk_(chunk), pos_(pos), tail_(tail) {}
        
        T& operator*() { return chunk_->data[pos_]; }
        
        Iterator& operator++() {
            pos_++;
            if (pos_ >= chunk_->size && chunk_->next != tail_) {
                chunk_ = chunk_->next;
                pos_ = 0;
            }
            return *this;
        }
        
        Iterator& operator--() {
            pos_--;
            if (pos_ < 0 && chunk_->prev->prev != nullptr) {
                chunk_ = chunk_->prev;
                pos_ = chunk_->size - 1;
            }
            return *this;
        }
        
        bool operator==(const Iterator& o) const {
            return chunk_ == o.chunk_ && pos_ == o.pos_;
        }
        
        bool operator!=(const Iterator& o) const {
            return !(*this == o);
        }
    };
    
    Iterator begin() {
        Chunk* first = head_->next;
        return Iterator(first, 0, tail_);
    }
    
    Iterator end() {
        Chunk* last = tail_->prev;
        return Iterator(last, last->size, tail_);
    }
};

// 预定义不同 chunk size
template <class T>
using UnrollLinkedList16 = UnrollLinkedList<T, 16>;
template <class T>
using UnrollLinkedList64 = UnrollLinkedList<T, 64>;
template <class T>
using UnrollLinkedList256 = UnrollLinkedList<T, 256>;
template <class T>
using UnrollLinkedList1024 = UnrollLinkedList<T, 1024>;
