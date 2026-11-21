#pragma once

#include <algorithm>
#include <cstddef>
#include <forward_list>
#include <mutex>
#include <shared_mutex>

namespace ts {

template <typename T>
class mutex_forward_list {
  private:
   std::forward_list<T> list_;
   size_t size_{0};
   mutable std::shared_mutex mutex_;

  public:
   mutex_forward_list() = default;

   mutex_forward_list(const mutex_forward_list&) = delete;
   mutex_forward_list& operator=(const mutex_forward_list&) = delete;

   void push_front(const T& value) {
      std::unique_lock lock(mutex_);
      list_.push_front(value);
      ++size_;
   }

   void pop_front() {
      std::unique_lock lock(mutex_);
      if (!list_.empty()) {
         list_.pop_front();
         --size_;
      }
   }

   bool contains(const T& value) const {
      std::shared_lock lock(mutex_);
      return std::find(list_.begin(), list_.end(), value) != list_.end();
   }

   size_t size() const {
      std::shared_lock lock(mutex_);
      return size_;
   }

   bool empty() const {
      std::shared_lock lock(mutex_);
      return list_.empty() && size_ == 0;
   }

   void clear() {
      std::unique_lock lock(mutex_);
      list_.clear();
      size_ = 0;
   }
};

}  // namespace ts
