#pragma once

#include <atomic>
#include <cstddef>

namespace ts {

template <typename T>
class lockfree_forward_list {
  private:
   struct Node {
      T data;
      std::atomic<Node*> next{nullptr};
      Node* alloc_next{nullptr};
   };

   std::atomic<Node*> head_{nullptr};
   std::atomic<Node*> allocated_head_{nullptr};
   std::atomic<size_t> size_{0};

   Node* allocate_node(const T& value) {
      Node* node = new Node{value};
      Node* old_alloc = allocated_head_.load(std::memory_order_relaxed);
      while (true) {
         node->alloc_next = old_alloc;
         if (allocated_head_.compare_exchange_weak(old_alloc, node,
                                                   std::memory_order_release,
                                                   std::memory_order_relaxed)) {
            break;
         }
      }
      return node;
   }

  public:
   lockfree_forward_list() = default;

   ~lockfree_forward_list() {
      Node* curr = allocated_head_.load(std::memory_order_relaxed);
      while (curr != nullptr) {
         Node* next = curr->alloc_next;
         delete curr;
         curr = next;
      }
   }

   lockfree_forward_list(const lockfree_forward_list&) = delete;
   lockfree_forward_list& operator=(const lockfree_forward_list&) = delete;

   void push_front(const T& value) {
      Node* new_node = allocate_node(value);
      Node* old_head = head_.load(std::memory_order_relaxed);
      while (true) {
         new_node->next.store(old_head, std::memory_order_relaxed);
         if (head_.compare_exchange_weak(old_head, new_node,
                                         std::memory_order_release,
                                         std::memory_order_relaxed)) {
            break;
         }
      }
      size_.fetch_add(1, std::memory_order_relaxed);
   }

   void pop_front() {
      Node* old_head = head_.load(std::memory_order_acquire);
      while (old_head != nullptr) {
         Node* next = old_head->next.load(std::memory_order_relaxed);
         if (head_.compare_exchange_weak(old_head, next,
                                         std::memory_order_acquire,
                                         std::memory_order_acquire)) {
            size_.fetch_sub(1, std::memory_order_relaxed);
            return;
         }
      }
   }

   bool contains(const T& value) const {
      Node* curr = head_.load(std::memory_order_acquire);
      while (curr != nullptr) {
         if (curr->data == value) {
            return true;
         }
         curr = curr->next.load(std::memory_order_acquire);
      }
      return false;
   }

   size_t size() const { return size_.load(std::memory_order_relaxed); }

   bool empty() const { return size() == 0; }

   void clear() {
      head_.store(nullptr, std::memory_order_release);
      size_.store(0, std::memory_order_relaxed);
   }
};

}  // namespace ts