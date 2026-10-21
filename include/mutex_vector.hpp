#pragma once

#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <vector>

namespace ts {

template <typename T>
class mutex_vector {
private:
    mutable std::shared_mutex mutex_;
    std::vector<T> data_;

public:
    mutex_vector() = default;

    mutex_vector(const mutex_vector&) = delete;
    mutex_vector& operator=(const mutex_vector&) = delete;

    void push_back(const T& value) {
        std::unique_lock lock(mutex_);
        data_.push_back(value);
    }

    void pop_back() {
        std::unique_lock lock(mutex_);
        if (!data_.empty()) {
            data_.pop_back();
        }
    }

    T get(size_t index) const {
        std::shared_lock lock(mutex_);
        return data_[index];
    }

    void set(size_t index, const T& value) {
        std::unique_lock lock(mutex_);
        if (index < data_.size()) {
            data_[index] = value;
        }
    }

    T& operator[](size_t index) {
        std::unique_lock lock(mutex_);
        return data_[index];
    }

    const T& operator[](size_t index) const {
        std::shared_lock lock(mutex_);
        return data_[index];
    }

    size_t size() const {
        std::shared_lock lock(mutex_);
        return data_.size();
    }

    bool empty() const {
        std::shared_lock lock(mutex_);
        return data_.empty();
    }

    void clear() {
        std::unique_lock lock(mutex_);
        data_.clear();
    }
};

} // namespace ts
