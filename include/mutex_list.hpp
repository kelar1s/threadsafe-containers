#pragma once

#include <algorithm>
#include <cstddef>
#include <list>
#include <mutex>
#include <shared_mutex>

namespace ts {

template <typename T>
class mutex_list {
 private:
  std::list<T> list_;
  mutable std::shared_mutex mutex_;

 public:
  mutex_list() = default;

  mutex_list(const mutex_list&) = delete;
  mutex_list& operator=(const mutex_list&) = delete;

  void push_front(const T& value) {
    std::unique_lock lock(mutex_);
    list_.push_front(value);
  }

  void push_back(const T& value) {
    std::unique_lock lock(mutex_);
    list_.push_back(value);
  }

  void pop_front() {
    std::unique_lock lock(mutex_);
    if (!list_.empty()) {
      list_.pop_front();
    }
  }

  void pop_back() {
    std::unique_lock lock(mutex_);
    if (!list_.empty()) {
      list_.pop_back();
    }
  }

  void remove(const T& value) {
    std::unique_lock lock(mutex_);
    list_.remove(value);
  }

  bool contains(const T& value) const {
    std::shared_lock lock(mutex_);
    return std::find(list_.begin(), list_.end(), value) != list_.end();
  }

  size_t size() const {
    std::shared_lock lock(mutex_);
    return list_.size();
  }

  bool empty() const {
    std::shared_lock lock(mutex_);
    return list_.empty();
  }

  void clear() {
    std::unique_lock lock(mutex_);
    list_.clear();
  }
};

}  // namespace ts
