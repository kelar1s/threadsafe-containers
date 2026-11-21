#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>

#include "../../utils/bench_engine.hpp"

namespace perf::ops {

template <typename VectorType>
double vector_push_back(int num_threads, int total_elements) {
   VectorType vec;

   double elapsed = bench::measure_ms([&]() {
      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int i = start; i < end; ++i) {
                                   vec.push_back(i);
                                }
                             });
   });

   assert(vec.size() == static_cast<size_t>(total_elements));
   return elapsed;
}

struct VectorSetup {
   template <typename VectorType>
   void operator()(VectorType& vec, int total_elements) const {
      for (int i = 0; i < total_elements; ++i) {
         vec.push_back(i);
      }
   }
};

struct VectorGetRun {
   template <typename VectorType>
   double operator()(const VectorType& vec, int num_threads,
                     int total_elements) const {
      std::atomic<long long> checksum{0};

      double elapsed = bench::measure_ms([&]() {
         bench::parallel_chunks(
             num_threads, total_elements, [&](int start, int end) {
                long long local_sum = 0;
                for (int i = start; i < end; ++i) {
                   local_sum += vec.get(static_cast<size_t>(i));
                }
                checksum.fetch_add(local_sum, std::memory_order_relaxed);
             });
      });

      assert(checksum.load() >= 0);
      return elapsed;
   }
};

struct VectorSetRun {
   template <typename VectorType>
   double operator()(VectorType& vec, int num_threads,
                     int total_elements) const {
      return bench::measure_ms([&]() {
         bench::parallel_chunks(num_threads, total_elements,
                                [&](int start, int end) {
                                   for (int i = start; i < end; ++i) {
                                      vec.set(static_cast<size_t>(i), i + 1);
                                   }
                                });
      });
   }
};

template <typename VectorType>
double vector_pop_back(int num_threads, int total_elements) {
   VectorType vec;
   for (int i = 0; i < total_elements; ++i) {
      vec.push_back(i);
   }

   double elapsed = bench::measure_ms([&]() {
      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int i = start; i < end; ++i) {
                                   vec.pop_back();
                                }
                             });
   });

   assert(vec.empty());
   return elapsed;
}

}  // namespace perf::ops