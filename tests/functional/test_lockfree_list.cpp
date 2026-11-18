#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "lockfree_list.hpp"

TEST(LockfreeList, PushAndContains) {
   ts::lockfree_list<int> list;
   constexpr int kThreads = 8;
   constexpr int kOps = 200;

   std::vector<std::thread> workers;
   for (int t = 0; t < kThreads; ++t) {
      workers.emplace_back([&list, t]() {
         for (int i = 0; i < kOps; ++i) {
            if (t % 2 == 0)
               list.push_front(t * kOps + i);
            else
               list.push_back(t * kOps + i);
         }
      });
   }
   for (auto& w : workers) w.join();

   const size_t expected_size = kThreads * kOps;
   EXPECT_EQ(list.size(), expected_size);

   workers.clear();
   for (int t = 0; t < kThreads; ++t) {
      workers.emplace_back([&list, t]() {
         for (int i = 0; i < kOps; ++i) {
            EXPECT_TRUE(list.contains(t * kOps + i));
         }
      });
   }
   for (auto& w : workers) w.join();

   EXPECT_FALSE(list.contains(-1));
}

TEST(LockfreeList, ConcurrentPopFrontAndBack) {
   ts::lockfree_list<int> list;
   constexpr int kInit = 20000;
   constexpr int kThreads = 8;

   for (int i = 0; i < kInit; ++i) list.push_back(i);

   std::vector<std::thread> threads;
   for (int t = 0; t < kThreads; ++t) {
      threads.emplace_back([&list, t]() {
         while (!list.empty()) {
            if (t % 2 == 0)
               list.pop_front();
            else
               list.pop_back();
         }
      });
   }
   for (auto& t : threads) t.join();

   EXPECT_TRUE(list.empty());
}

TEST(LockfreeList, ConcurrentRemove) {
   ts::lockfree_list<int> list;
   constexpr int kThreads = 8;
   constexpr int kOps = 100;

   for (int t = 0; t < kThreads; ++t) {
      for (int i = 0; i < kOps; ++i) {
         list.push_back(t * kOps + i);
      }
   }

   std::vector<std::thread> threads;
   for (int t = 0; t < kThreads; ++t) {
      threads.emplace_back([&list, t]() {
         for (int i = 0; i < kOps; ++i) {
            list.remove(t * kOps + i);
         }
      });
   }
   for (auto& t : threads) t.join();

   EXPECT_TRUE(list.empty());
}

TEST(LockfreeList, ConcurrentSize) {
   ts::lockfree_list<int> list;
   constexpr size_t kMaxOps = 15000;

   std::atomic<bool> writer_done{false};

   std::thread writer([&]() {
      for (size_t i = 0; i < kMaxOps; ++i) {
         list.push_back(static_cast<int>(i));
         if (i % 3 == 0) list.pop_front();
      }
      writer_done.store(true);
   });

   std::thread reader([&]() {
      while (!writer_done.load()) {
         EXPECT_LE(list.size(), kMaxOps);
      }
   });

   writer.join();
   reader.join();

   const size_t expected = kMaxOps - (kMaxOps / 3);
   EXPECT_EQ(list.size(), expected);
}
