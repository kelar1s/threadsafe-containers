#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "lockfree_forward_list.hpp"

TEST(LockfreeForwardList, PushFrontAndContains) {
   ts::lockfree_forward_list<int> flist;
   constexpr int kThreads = 8;
   constexpr int kOps     = 200;

   std::vector<std::thread> workers;
   for (int t = 0; t < kThreads; ++t) {
      workers.emplace_back([&flist, t]() {
         for (int i = 0; i < kOps; ++i) flist.push_front(t * kOps + i);
      });
   }
   for (auto& w : workers) w.join();

   const size_t expected_size = kThreads * kOps;
   EXPECT_EQ(flist.size(), expected_size);

   workers.clear();
   for (int t = 0; t < kThreads; ++t) {
      workers.emplace_back([&flist, t]() {
         for (int i = 0; i < kOps; ++i) {
            EXPECT_TRUE(flist.contains(t * kOps + i));
         }
      });
   }
   for (auto& w : workers) w.join();

   EXPECT_FALSE(flist.contains(-1));
   EXPECT_FALSE(flist.contains(static_cast<int>(expected_size + 100)));
}

TEST(LockfreeForwardList, ConcurrentPopFront) {
   ts::lockfree_forward_list<int> flist;
   constexpr int kInit    = 20000;
   constexpr int kThreads = 8;

   for (int i = 0; i < kInit; ++i) flist.push_front(i);

   std::atomic<int> total_popped{0};
   std::vector<std::thread> threads;
   for (int t = 0; t < kThreads; ++t) {
      threads.emplace_back([&flist, &total_popped]() {
         while (!flist.empty()) {
            flist.pop_front();
            total_popped.fetch_add(1, std::memory_order_relaxed);
         }
      });
   }
   for (auto& t : threads) t.join();

   EXPECT_TRUE(flist.empty());
   EXPECT_FALSE(flist.contains(0));
   std::cout << "[info] total_popped = " << total_popped.load() << '\n';
}

TEST(LockfreeForwardList, ConcurrentSize) {
   ts::lockfree_forward_list<int> flist;
   constexpr size_t kMaxOps = 15000;

   std::atomic<bool>   writer_done{false};
   std::atomic<size_t> checks{0};

   std::thread writer([&]() {
      for (size_t i = 0; i < kMaxOps; ++i) {
         flist.push_front(static_cast<int>(i));
         if (i % 3 == 0) flist.pop_front();
      }
      writer_done.store(true);
   });

   std::thread reader([&]() {
      while (!writer_done.load()) {
         EXPECT_LE(flist.size(), kMaxOps);
         checks.fetch_add(1, std::memory_order_relaxed);
      }
   });

   writer.join();
   reader.join();

   const size_t expected = kMaxOps - (kMaxOps / 3);
   EXPECT_EQ(flist.size(), expected);
   EXPECT_FALSE(flist.empty());
   EXPECT_GT(checks.load(), 0);
}

TEST(LockfreeForwardList, Clear) {
   ts::lockfree_forward_list<int> flist;
   for (int i = 0; i < 5000; ++i) flist.push_front(i);

   EXPECT_EQ(flist.size(), 5000);
   EXPECT_FALSE(flist.empty());

   flist.clear();
   EXPECT_TRUE(flist.empty());
   EXPECT_EQ(flist.size(), 0);
   EXPECT_FALSE(flist.contains(100));

   flist.push_front(42);
   EXPECT_FALSE(flist.empty());
   EXPECT_EQ(flist.size(), 1);
   EXPECT_TRUE(flist.contains(42));
}
