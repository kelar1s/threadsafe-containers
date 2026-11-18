#pragma once

#include <cassert>
#include <chrono>
#include <thread>
#include <vector>

namespace perf {

template <typename VectorType>
double vector_workload(int num_threads, int total_elements) {
   VectorType vec;
   int chunk = total_elements / num_threads;

   auto start_time = std::chrono::high_resolution_clock::now();

   std::vector<std::thread> threads;
   for (int t = 0; t < num_threads; ++t) {
      int start_idx = t * chunk;
      int end_idx = (t == num_threads - 1) ? total_elements : (t + 1) * chunk;
      int ops = end_idx - start_idx;

      threads.emplace_back([&vec, ops]() {
         for (int i = 0; i < ops; ++i) {
            vec.push_back(0);
         }
      });
   }
   for (auto& th : threads) th.join();

   assert(vec.size() == static_cast<size_t>(total_elements));

   threads.clear();
   for (int t = 0; t < num_threads; ++t) {
      size_t start_idx = static_cast<size_t>(t * chunk);
      size_t end_idx = (t == num_threads - 1)
                           ? static_cast<size_t>(total_elements)
                           : static_cast<size_t>((t + 1) * chunk);

      threads.emplace_back([&vec, start_idx, end_idx]() {
         for (size_t idx = start_idx; idx < end_idx; ++idx) {
            vec[idx] = static_cast<int>(idx + 1);
            vec.set(idx, vec.get(idx) + 1);
         }
      });
   }
   for (auto& th : threads) th.join();

   for (size_t i = 0; i < static_cast<size_t>(total_elements); ++i) {
      int expected = static_cast<int>(i + 2);
      assert(vec.get(i) == expected);
   }

   vec.clear();
   assert(vec.empty());

   auto end_time = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
   return elapsed.count();
}

}  // namespace perf