#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace bench {

using WorkloadFn = std::function<double(int, int)>;

template <typename Fn>
inline double measure_ms(Fn&& fn) {
   auto start = std::chrono::high_resolution_clock::now();
   fn();
   auto end = std::chrono::high_resolution_clock::now();
   return std::chrono::duration<double, std::milli>(end - start).count();
}

template <typename Fn>
inline void parallel_chunks(int num_threads, int total_items, Fn&& fn) {
   int chunk = total_items / num_threads;
   std::vector<std::thread> threads;
   threads.reserve(num_threads);

   for (int t = 0; t < num_threads; ++t) {
      int start = t * chunk;
      int end = (t == num_threads - 1) ? total_items : (t + 1) * chunk;
      threads.emplace_back([&fn, start, end]() { fn(start, end); });
   }
   for (auto& th : threads) th.join();
}

inline double measure_mean(const WorkloadFn& single_run, int num_threads,
                           int total_elements, int num_runs = 5) {
   std::vector<double> times(num_runs);
   for (int i = 0; i < num_runs; ++i) {
      times[i] = single_run(num_threads, total_elements);
   }

   if (num_runs <= 1) {
      return times[0];
   }

   std::sort(times.begin(), times.end());

   double sum = 0.0;
   for (int i = 0; i < num_runs - 1; ++i) {
      sum += times[i];
   }
   return sum / static_cast<double>(num_runs - 1);
}

inline void print_header(const std::string& container_name,
                         const std::string& operation_name) {
   std::cout << "\n============================================================"
                "========\n";
   std::cout << "CONTAINER: " << container_name
             << " | OPERATION: " << operation_name << "\n";
   std::cout << "=============================================================="
                "======\n\n";
}

inline void print_table_columns(const std::string& container_name,
                                int total_elements, int num_runs) {
   std::cout << "Benchmark: " << total_elements << " elements (" << num_runs
             << " runs)\n";
   std::cout << std::left << std::setw(10) << "Threads" << std::setw(24)
             << ("Mutex " + container_name + " (ms)") << std::setw(26)
             << ("Lock-Free " + container_name + " (ms)") << "Speedup\n";
   std::cout << std::string(70, '-') << "\n";
}

inline void run_comparison_table(
    const std::string& container_name, const std::string& operation_name,
    const WorkloadFn& mutex_workload, const WorkloadFn& lockfree_workload,
    const std::vector<int>& element_counts = {100000, 500000, 1000000},
    const std::vector<int>& thread_counts = {1, 2, 4, 6}, int num_runs = 5) {
   print_header(container_name, operation_name);

   for (int total_elements : element_counts) {
      print_table_columns(container_name, total_elements, num_runs);

      for (int threads : thread_counts) {
         double mutex_time =
             measure_mean(mutex_workload, threads, total_elements, num_runs);
         double lockfree_time =
             measure_mean(lockfree_workload, threads, total_elements, num_runs);
         double speedup =
             (lockfree_time > 0.0) ? (mutex_time / lockfree_time) : 0.0;

         std::cout << std::left << std::setw(10) << threads << std::fixed
                   << std::setprecision(2) << std::setw(24) << mutex_time
                   << std::setw(26) << lockfree_time << speedup << "x\n";
      }
      std::cout << "\n";
   }
}

template <typename MutexContainer, typename LockFreeContainer, typename SetupFn,
          typename RunFn>
inline void run_setup_comparison_table(
    const std::string& container_name, const std::string& operation_name,
    SetupFn setup_fn, RunFn run_fn,
    const std::vector<int>& element_counts = {100000, 500000, 1000000},
    const std::vector<int>& thread_counts = {1, 2, 4, 6}, int num_runs = 5) {
   print_header(container_name, operation_name);

   for (int total_elements : element_counts) {
      MutexContainer mutex_cont;
      LockFreeContainer lockfree_cont;

      setup_fn(mutex_cont, total_elements);
      setup_fn(lockfree_cont, total_elements);

      print_table_columns(container_name, total_elements, num_runs);

      for (int threads : thread_counts) {
         double mutex_time = measure_mean(
             [&](int th, int n) { return run_fn(mutex_cont, th, n); }, threads,
             total_elements, num_runs);

         double lockfree_time = measure_mean(
             [&](int th, int n) { return run_fn(lockfree_cont, th, n); },
             threads, total_elements, num_runs);

         double speedup =
             (lockfree_time > 0.0) ? (mutex_time / lockfree_time) : 0.0;

         std::cout << std::left << std::setw(10) << threads << std::fixed
                   << std::setprecision(2) << std::setw(24) << mutex_time
                   << std::setw(26) << lockfree_time << speedup << "x\n";
      }
      std::cout << "\n";
   }
}

}  // namespace bench