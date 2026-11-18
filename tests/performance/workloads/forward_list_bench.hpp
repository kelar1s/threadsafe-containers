#pragma once

#include <cassert>
#include <chrono>
#include <thread>
#include <vector>

namespace perf {

template <typename ListType>
double forward_list_workload(int num_threads, int total_elements) {
  ListType flist;
  int chunk = total_elements / num_threads;

  auto start_time = std::chrono::high_resolution_clock::now();

  std::vector<std::thread> threads;
  for (int t = 0; t < num_threads; ++t) {
    int start_val = t * chunk;
    int end_val = (t == num_threads - 1) ? total_elements : (t + 1) * chunk;

    threads.emplace_back([&flist, start_val, end_val]() {
      for (int val = start_val; val < end_val; ++val) {
        flist.push_front(val);
      }
    });
  }
  for (auto& th : threads) th.join();

  assert(flist.size() == static_cast<size_t>(total_elements));

  int searches_per_thread = 60 / num_threads;
  threads.clear();
  for (int t = 0; t < num_threads; ++t) {
    int end_val = (t == num_threads - 1) ? total_elements : (t + 1) * chunk;
    threads.emplace_back([&flist, end_val, searches_per_thread]() {
      for (int i = 0; i < searches_per_thread; ++i) {
        bool found = flist.contains(end_val - 1 - i);
        assert(found);
      }
    });
  }
  for (auto& th : threads) th.join();

  int pop_per_thread = (total_elements / 2) / num_threads;
  threads.clear();
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([&flist, pop_per_thread]() {
      for (int i = 0; i < pop_per_thread; ++i) {
        flist.pop_front();
      }
    });
  }
  for (auto& th : threads) th.join();

  flist.clear();
  assert(flist.empty());

  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
  return elapsed.count();
}

}  // namespace perf
