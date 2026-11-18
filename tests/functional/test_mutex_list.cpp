#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "mutex_list.hpp"

TEST(MutexList, PushAndContains) {
  ts::mutex_list<int> lst;
  constexpr int kThreads = 8;
  constexpr int kOps = 200;

  std::vector<std::thread> workers;
  for (int t = 0; t < kThreads; ++t) {
    workers.emplace_back([&lst, t]() {
      for (int i = 0; i < kOps; ++i) {
        if (t % 2 == 0)
          lst.push_front(t * kOps + i);
        else
          lst.push_back(t * kOps + i);
      }
    });
  }
  for (auto& w : workers) w.join();

  const size_t expected_size = kThreads * kOps;
  EXPECT_EQ(lst.size(), expected_size);

  workers.clear();
  for (int t = 0; t < kThreads; ++t) {
    workers.emplace_back([&lst, t]() {
      for (int i = 0; i < kOps; ++i) {
        EXPECT_TRUE(lst.contains(t * kOps + i));
      }
    });
  }
  for (auto& w : workers) w.join();

  EXPECT_FALSE(lst.contains(-1));
}

TEST(MutexList, ConcurrentPopFrontAndBack) {
  ts::mutex_list<int> lst;
  constexpr int kInit = 20000;
  constexpr int kThreads = 8;

  for (int i = 0; i < kInit; ++i) lst.push_back(i);

  std::vector<std::thread> threads;
  for (int t = 0; t < kThreads; ++t) {
    threads.emplace_back([&lst, t]() {
      while (!lst.empty()) {
        if (t % 2 == 0)
          lst.pop_front();
        else
          lst.pop_back();
      }
    });
  }
  for (auto& t : threads) t.join();

  EXPECT_TRUE(lst.empty());
}

TEST(MutexList, ConcurrentRemove) {
  ts::mutex_list<int> lst;
  constexpr int kThreads = 8;
  constexpr int kOps = 100;

  for (int t = 0; t < kThreads; ++t) {
    for (int i = 0; i < kOps; ++i) {
      lst.push_back(t * kOps + i);
    }
  }

  std::vector<std::thread> threads;
  for (int t = 0; t < kThreads; ++t) {
    threads.emplace_back([&lst, t]() {
      for (int i = 0; i < kOps; ++i) {
        lst.remove(t * kOps + i);
      }
    });
  }
  for (auto& t : threads) t.join();

  EXPECT_TRUE(lst.empty());
}

TEST(MutexList, ConcurrentSize) {
  ts::mutex_list<int> lst;
  constexpr size_t kMaxOps = 15000;

  std::atomic<bool> writer_done{false};

  std::thread writer([&]() {
    for (size_t i = 0; i < kMaxOps; ++i) {
      lst.push_back(static_cast<int>(i));
      if (i % 3 == 0) lst.pop_front();
    }
    writer_done.store(true);
  });

  std::thread reader([&]() {
    while (!writer_done.load()) {
      EXPECT_LE(lst.size(), kMaxOps);
    }
  });

  writer.join();
  reader.join();

  const size_t expected = kMaxOps - (kMaxOps / 3);
  EXPECT_EQ(lst.size(), expected);
}
