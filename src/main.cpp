#include <atomic>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "lockfree_vector.hpp"
#include "mutex_vector.hpp"

template <typename VectorType>
double measure_single_run(int num_threads, int total_elements) {
   VectorType vec;
   int chunk = total_elements / num_threads;

   auto start_time = std::chrono::high_resolution_clock::now();

   std::vector<std::thread> threads;
   for (int t = 0; t < num_threads; ++t) {
      int start_idx = t * chunk;
      int end_idx = (t == num_threads - 1) ? total_elements : (t + 1) * chunk;
      int ops = end_idx - start_idx;

      threads.push_back(std::thread([&vec, ops]() {
         for (int i = 0; i < ops; ++i) {
            vec.push_back(0);
         }
      }));
   }
   for (auto& th : threads) {
      th.join();
   }

   assert(vec.size() == static_cast<size_t>(total_elements));

   threads.clear();
   for (int t = 0; t < num_threads; ++t) {
      size_t start_idx = static_cast<size_t>(t * chunk);
      size_t end_idx = (t == num_threads - 1)
                           ? static_cast<size_t>(total_elements)
                           : static_cast<size_t>((t + 1) * chunk);

      threads.push_back(std::thread([&vec, start_idx, end_idx]() {
         for (size_t idx = start_idx; idx < end_idx; ++idx) {
            vec[idx] = static_cast<int>(idx + 1);
            vec.set(idx, vec.get(idx) + 1);
         }
      }));
   }
   for (auto& th : threads) {
      th.join();
   }

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

template <typename VectorType>
double measure_performance(int num_threads, int total_elements,
                           int num_runs = 5) {
   std::vector<double> times;

   for (int i = 0; i < num_runs; ++i) {
      times.push_back(
          measure_single_run<VectorType>(num_threads, total_elements));
   }

   std::sort(times.begin(), times.end());

   double sum = 0.0;
   for (int i = 1; i < num_runs - 1; ++i) {
      sum += times[i];
   }
   return sum / static_cast<double>(num_runs - 2);
}

void run_benchmarks() {
   const std::vector<int> element_counts = {100000, 500000, 1000000};
   const std::vector<int> thread_counts = {1, 2, 4, 6};
   const int num_runs = 5;

   std::cout << std::endl << std::endl;

   for (int total_elements : element_counts) {
      std::cout << "Benchmark: " << total_elements << " elements (" << num_runs
                << " runs)" << std::endl;
      std::cout << std::left << std::setw(10) << "Threads " << std::setw(22)
                << "Mutex Vector (ms) " << std::setw(22)
                << "Lock-Free Vector (ms) " << "Speedup" << std::endl;
      std::cout << std::string(65, '-') << std::endl;

      for (int threads : thread_counts) {
         double mutex_time = measure_performance<ts::mutex_vector<int>>(
             threads, total_elements, num_runs);
         double lockfree_time = measure_performance<ts::lockfree_vector<int>>(
             threads, total_elements, num_runs);
         double speedup = mutex_time / lockfree_time;

         std::cout << std::left << std::setw(10) << threads << std::fixed
                   << std::setprecision(2) << std::setw(22) << mutex_time
                   << std::setw(22) << lockfree_time << speedup << "x"
                   << std::endl;
      }
      std::cout << std::endl << std::endl;
   }
}

int main() {
   run_benchmarks();

   return 0;
}
