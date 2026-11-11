#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "lockfree_vector.hpp"

TEST(LockfreeVector, ConcurrentPushBack) {
   ts::lockfree_vector<int> vec;
   constexpr int kThreads = 8;
   constexpr int kOps     = 10000;

   std::vector<std::thread> workers;
   for (int t = 0; t < kThreads; ++t) {
      workers.emplace_back([&vec, t]() {
         for (int i = 0; i < kOps; ++i) vec.push_back(t * kOps + i);
      });
   }
   for (auto& w : workers) w.join();

   EXPECT_EQ(vec.size(), static_cast<size_t>(kThreads * kOps));
}

TEST(LockfreeVector, ConcurrentIndexSetGet) {
   ts::lockfree_vector<int> vec;
   constexpr int kCount   = 5000;
   constexpr int kThreads = 4;

   for (int i = 0; i < kCount; ++i) vec.push_back(i);

   std::vector<std::thread> threads;
   for (int t = 0; t < kThreads; ++t) {
      threads.emplace_back([&vec, t]() {
         const int chunk = kCount / kThreads;
         const int start = t * chunk;
         const int end   = start + chunk;
         for (int i = start; i < end; ++i) {
            vec[i] += 10;
            vec.set(i, vec.get(i) + 5);
         }
      });
   }
   for (auto& th : threads) th.join();

   for (int i = 0; i < kCount; ++i) {
      EXPECT_EQ(vec[i], i + 15) << "index = " << i;
   }
}

TEST(LockfreeVector, ConcurrentPopBack) {
   ts::lockfree_vector<int> vec;
   constexpr int kInit    = 20000;
   constexpr int kThreads = 4;

   for (int i = 0; i < kInit; ++i) vec.push_back(i);

   std::atomic<int> total_popped{0};
   std::vector<std::thread> threads;
   for (int t = 0; t < kThreads; ++t) {
      threads.emplace_back([&vec, &total_popped]() {
         while (!vec.empty()) {
            vec.pop_back();
            total_popped.fetch_add(1, std::memory_order_relaxed);
         }
      });
   }
   for (auto& t : threads) t.join();

   EXPECT_TRUE(vec.empty());
   std::cout << "[info] total_popped = " << total_popped.load() << '\n';
}

TEST(LockfreeVector, ConcurrentSizeEmpty) {
   ts::lockfree_vector<int> vec;
   constexpr size_t kMaxOps = 15000;

   std::atomic<bool>   writer_done{false};
   std::atomic<size_t> checks{0};

   std::thread writer([&]() {
      for (size_t i = 0; i < kMaxOps; ++i) {
         vec.push_back(static_cast<int>(i));
         if (i % 3 == 0) vec.pop_back();
      }
      writer_done.store(true);
   });

   std::thread reader([&]() {
      while (!writer_done.load()) {
         EXPECT_LE(vec.size(), kMaxOps);
         checks.fetch_add(1, std::memory_order_relaxed);
      }
   });

   writer.join();
   reader.join();

   const size_t expected = kMaxOps - (kMaxOps / 3);
   EXPECT_EQ(vec.size(), expected);
   EXPECT_FALSE(vec.empty());
   EXPECT_GT(checks.load(), 0);
}
