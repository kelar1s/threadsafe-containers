#pragma once

#include <atomic>
#include <cstddef>

namespace ts {

template <typename T>
class lockfree_list {
  private:
   struct Node {
      T data{};
      std::atomic<Node*> prev{nullptr};
      std::atomic<Node*> next{nullptr};
      std::atomic<bool> deleted{false};
      Node* alloc_next{nullptr};
   };

   std::atomic<Node*> head_{nullptr};
   std::atomic<Node*> tail_{nullptr};
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

   void unlink_deleted(Node* curr) {
      Node* prev = curr->prev.load(std::memory_order_acquire);
      Node* next = curr->next.load(std::memory_order_acquire);
      if (prev != nullptr && next != nullptr) {
         Node* expected_next = curr;
         prev->next.compare_exchange_strong(expected_next, next,
                                            std::memory_order_release,
                                            std::memory_order_relaxed);

         Node* expected_prev = curr;
         next->prev.compare_exchange_strong(expected_prev, prev,
                                            std::memory_order_release,
                                            std::memory_order_relaxed);
      }
   }

   void clean_head() {
      Node* curr = head_.load(std::memory_order_acquire);
      while (curr != nullptr && curr->deleted.load(std::memory_order_acquire)) {
         if (curr == tail_.load(std::memory_order_acquire)) break;
         Node* n = curr->next.load(std::memory_order_acquire);
         if (n == nullptr) break;
         head_.compare_exchange_weak(curr, n, std::memory_order_release,
                                     std::memory_order_relaxed);
         curr = head_.load(std::memory_order_acquire);
      }
   }

   void clean_tail() {
      Node* curr = tail_.load(std::memory_order_acquire);
      while (curr != nullptr && curr->deleted.load(std::memory_order_acquire)) {
         if (curr == head_.load(std::memory_order_acquire)) break;
         Node* p = curr->prev.load(std::memory_order_acquire);
         if (p == nullptr) break;
         tail_.compare_exchange_weak(curr, p, std::memory_order_release,
                                     std::memory_order_relaxed);
         curr = tail_.load(std::memory_order_acquire);
      }
   }

  public:
   lockfree_list() {
      Node* dummy = allocate_node(T{});
      dummy->deleted.store(true, std::memory_order_relaxed);
      head_.store(dummy, std::memory_order_relaxed);
      tail_.store(dummy, std::memory_order_relaxed);
   }

   ~lockfree_list() {
      Node* curr = allocated_head_.load(std::memory_order_relaxed);
      while (curr != nullptr) {
         Node* next = curr->alloc_next;
         delete curr;
         curr = next;
      }
   }

   lockfree_list(const lockfree_list&) = delete;
   lockfree_list& operator=(const lockfree_list&) = delete;

   void push_front(const T& value) {
      Node* new_node = allocate_node(value);
      while (true) {
         Node* first = head_.load(std::memory_order_acquire);
         Node* p = first->prev.load(std::memory_order_acquire);

         if (p != nullptr && !p->deleted.load(std::memory_order_acquire)) {
            head_.compare_exchange_weak(first, p, std::memory_order_release,
                                        std::memory_order_relaxed);
            continue;
         }

         new_node->next.store(first, std::memory_order_relaxed);
         if (first->prev.compare_exchange_weak(p, new_node,
                                               std::memory_order_release,
                                               std::memory_order_relaxed)) {
            head_.compare_exchange_strong(first, new_node,
                                          std::memory_order_release,
                                          std::memory_order_relaxed);
            size_.fetch_add(1, std::memory_order_relaxed);
            return;
         }
      }
   }

   void push_back(const T& value) {
      Node* new_node = allocate_node(value);
      while (true) {
         Node* last = tail_.load(std::memory_order_acquire);
         Node* n = last->next.load(std::memory_order_acquire);

         if (n != nullptr && !n->deleted.load(std::memory_order_acquire)) {
            tail_.compare_exchange_weak(last, n, std::memory_order_release,
                                        std::memory_order_relaxed);
            continue;
         }

         new_node->prev.store(last, std::memory_order_relaxed);
         if (last->next.compare_exchange_weak(n, new_node,
                                              std::memory_order_release,
                                              std::memory_order_relaxed)) {
            tail_.compare_exchange_strong(last, new_node,
                                          std::memory_order_release,
                                          std::memory_order_relaxed);
            size_.fetch_add(1, std::memory_order_relaxed);
            return;
         }
      }
   }

   void pop_front() {
      Node* curr = head_.load(std::memory_order_acquire);
      while (curr != nullptr) {
         Node* next = curr->next.load(std::memory_order_acquire);
         if (!curr->deleted.load(std::memory_order_acquire)) {
            bool expected = false;
            if (curr->deleted.compare_exchange_strong(
                    expected, true, std::memory_order_acq_rel,
                    std::memory_order_relaxed)) {
               size_.fetch_sub(1, std::memory_order_relaxed);
               if (next != nullptr) {
                  head_.compare_exchange_weak(curr, next,
                                              std::memory_order_release,
                                              std::memory_order_relaxed);
               }
               return;
            }
         }
         if (next == nullptr || curr == tail_.load(std::memory_order_acquire))
            return;
         head_.compare_exchange_weak(curr, next, std::memory_order_release,
                                     std::memory_order_relaxed);
         curr = next;
      }
   }

   void pop_back() {
      Node* curr = tail_.load(std::memory_order_acquire);
      while (curr != nullptr) {
         Node* prev = curr->prev.load(std::memory_order_acquire);
         if (!curr->deleted.load(std::memory_order_acquire)) {
            bool expected = false;
            if (curr->deleted.compare_exchange_strong(
                    expected, true, std::memory_order_acq_rel,
                    std::memory_order_relaxed)) {
               size_.fetch_sub(1, std::memory_order_relaxed);
               if (prev != nullptr) {
                  tail_.compare_exchange_weak(curr, prev,
                                              std::memory_order_release,
                                              std::memory_order_relaxed);
               }
               return;
            }
         }
         if (prev == nullptr || curr == head_.load(std::memory_order_acquire))
            return;
         tail_.compare_exchange_weak(curr, prev, std::memory_order_release,
                                     std::memory_order_relaxed);
         curr = prev;
      }
   }

   void remove(const T& value) {
      clean_head();
      Node* curr = head_.load(std::memory_order_acquire);
      while (curr != nullptr) {
         bool is_deleted = curr->deleted.load(std::memory_order_acquire);
         if (!is_deleted && curr->data == value) {
            bool expected = false;
            if (curr->deleted.compare_exchange_strong(
                    expected, true, std::memory_order_acq_rel,
                    std::memory_order_relaxed)) {
               size_.fetch_sub(1, std::memory_order_relaxed);
               unlink_deleted(curr);
            }
         } else if (is_deleted) {
            unlink_deleted(curr);
         }
         curr = curr->next.load(std::memory_order_acquire);
      }
      clean_head();
      clean_tail();
   }

   bool contains(const T& value) const {
      Node* curr = head_.load(std::memory_order_acquire);
      while (curr != nullptr) {
         if (!curr->deleted.load(std::memory_order_acquire)) {
            if (curr->data == value) return true;
         }
         curr = curr->next.load(std::memory_order_acquire);
      }
      return false;
   }

   size_t size() const { return size_.load(std::memory_order_relaxed); }

   bool empty() const { return size() == 0; }

   void clear() {
      Node* dummy = allocate_node(T{});
      dummy->deleted.store(true, std::memory_order_relaxed);
      head_.store(dummy, std::memory_order_release);
      tail_.store(dummy, std::memory_order_release);
      size_.store(0, std::memory_order_relaxed);
   }
};

}  // namespace ts
