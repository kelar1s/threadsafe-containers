#pragma once

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace bench {

using WorkloadFn = std::function<double(int, int)>;

inline double measure_trimmed_mean(const WorkloadFn& single_run,
                                   int num_threads, int total_elements,
                                   int num_runs = 5) {
   std::vector<double> times(num_runs);
   for (int i = 0; i < num_runs; ++i) {
      times[i] = single_run(num_threads, total_elements);
   }

   std::sort(times.begin(), times.end());

   double sum = 0.0;
   for (int i = 1; i < num_runs - 1; ++i) {
      sum += times[i];
   }
   return sum / static_cast<double>(num_runs - 2);
}

inline void run_comparison_table(const std::string& container_name,
                                 const WorkloadFn& mutex_workload,
                                 const WorkloadFn& lockfree_workload) {
   const std::vector<int> element_counts = {100000, 500000, 1000000};
   const std::vector<int> thread_counts = {1, 2, 4, 6};
   const int num_runs = 5;

   std::cout << "\n============================================================"
                "========\n";
   std::cout << "CONTAINER: " << container_name << "\n";
   std::cout << "=============================================================="
                "======\n\n";

   for (int total_elements : element_counts) {
      std::cout << "Benchmark: " << total_elements << " elements (" << num_runs
                << " runs)\n";
      std::cout << std::left << std::setw(10) << "Threads" << std::setw(24)
                << ("Mutex " + container_name + " (ms)") << std::setw(26)
                << ("Lock-Free " + container_name + " (ms)") << "Speedup\n";
      std::cout << std::string(70, '-') << "\n";

      for (int threads : thread_counts) {
         double mutex_time = measure_trimmed_mean(mutex_workload, threads,
                                                  total_elements, num_runs);
         double lockfree_time = measure_trimmed_mean(lockfree_workload, threads,
                                                     total_elements, num_runs);
         double speedup = mutex_time / lockfree_time;

         std::cout << std::left << std::setw(10) << threads << std::fixed
                   << std::setprecision(2) << std::setw(24) << mutex_time
                   << std::setw(26) << lockfree_time << speedup << "x\n";
      }
      std::cout << "\n";
   }
}

}  // namespace bench
