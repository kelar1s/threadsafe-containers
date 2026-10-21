#pragma once

#include <atomic>
#include <cstddef>

namespace ts {

template <typename T>
class lockfree_vector {
private:
    static const size_t BLOCK_SIZE = 1024;
    static const size_t MAX_BLOCKS = 1024;

    std::atomic<T*> blocks_[MAX_BLOCKS];
    std::atomic<size_t> size_{0};

    T* get_or_allocate_block(size_t block_idx) {
        T* block = blocks_[block_idx].load(std::memory_order_acquire);
        if (block == nullptr) {
            T* new_block = new T[BLOCK_SIZE];
            if (blocks_[block_idx].compare_exchange_strong(block, new_block)) {
                block = new_block;
            } else {
                delete[] new_block;
            }
        }
        return block;
    }

public:
    lockfree_vector() {
        for (size_t i = 0; i < MAX_BLOCKS; ++i) {
            blocks_[i].store(nullptr, std::memory_order_relaxed);
        }
        blocks_[0].store(new T[BLOCK_SIZE], std::memory_order_relaxed);
    }

    ~lockfree_vector() {
        for (size_t i = 0; i < MAX_BLOCKS; ++i) {
            T* block = blocks_[i].load(std::memory_order_relaxed);
            if (block != nullptr) {
                delete[] block;
            }
        }
    }

    lockfree_vector(const lockfree_vector&) = delete;
    lockfree_vector& operator=(const lockfree_vector&) = delete;

    void push_back(const T& value) {
        size_t index = size_.fetch_add(1);
        size_t block_idx = index / BLOCK_SIZE;
        size_t offset = index % BLOCK_SIZE;

        T* block = get_or_allocate_block(block_idx);
        block[offset] = value;
    }

    void pop_back() {
        size_t current = size_.load();
        while (current > 0) {
            if (size_.compare_exchange_weak(current, current - 1)) {
                break;
            }
        }
    }

    T& operator[](size_t index) {
        T* block = blocks_[index / BLOCK_SIZE].load(std::memory_order_relaxed);
        return block[index % BLOCK_SIZE];
    }

    const T& operator[](size_t index) const {
        const T* block = blocks_[index / BLOCK_SIZE].load(std::memory_order_relaxed);
        return block[index % BLOCK_SIZE];
    }

    T get(size_t index) const {
        const T* block = blocks_[index / BLOCK_SIZE].load(std::memory_order_relaxed);
        return block[index % BLOCK_SIZE];
    }

    void set(size_t index, const T& value) {
        T* block = blocks_[index / BLOCK_SIZE].load(std::memory_order_relaxed);
        block[index % BLOCK_SIZE] = value;
    }

    size_t size() const {
        return size_.load();
    }

    bool empty() const {
        return size() == 0;
    }

    void clear() {
        size_.store(0);
    }
};

} // namespace ts
