#pragma once

#include <mutex>
#include <shared_mutex>
#include <vector>

namespace ts {

template <typename T>
class vector {
  private:
   mutable std::shared_mutex mutex_;
   std::vector<T> data_;

  public:
   vector() = default;

   vector(const vector&) = delete;
   vector& operator=(const vector&) = delete;

   void push_back(const T& value) {
      std::unique_lock lock(mutex_);
      data_.push_back(value);
   }

   bool pop_back(T& out_value) {
      std::unique_lock lock(mutex_);
      if (data_.empty()) {
         return false;
      }
      out_value = data_.back();
      data_.pop_back();
      return true;
   }

   bool get(size_t index, T& out_value) const {
      std::shared_lock lock(mutex_);
      if (index < data_.size()) {
         out_value = data_[index];
         return true;
      }
      return false;
   }

   size_t size() const {
      std::shared_lock lock(mutex_);
      return data_.size();
   }

   bool empty() const {
      std::shared_lock lock(mutex_);
      return data_.empty();
   }
};

}  // namespace ts