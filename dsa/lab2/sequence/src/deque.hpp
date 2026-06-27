#pragma once

// CRTP 基类，提供 deque 接口的默认实现
template<class Derived, class T>
class DequeInterface {
public:
    void push_front(T x) {
        static_cast<Derived*>(this)->insert(0, x);
    }
    
    void push_back(T x) {
        auto* self = static_cast<Derived*>(this);
        self->insert(self->size(), x);
    }
    
    void pop_front() {
        static_cast<Derived*>(this)->remove(0);
    }
    
    void pop_back() {
        auto* self = static_cast<Derived*>(this);
        self->remove(self->size() - 1);
    }
    
    T front() {
        return static_cast<Derived*>(this)->query(0);
    }
    
    T back() {
        auto* self = static_cast<Derived*>(this);
        return self->query(self->size() - 1);
    }
};
