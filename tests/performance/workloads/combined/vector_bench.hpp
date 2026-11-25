#pragma once

#include <cassert>
#include <cstddef>

#include "../../utils/bench_engine.hpp"

namespace perf {

template <typename VectorType>
double vector_workload(int num_threads, int total_elements) {
   VectorType vec;

   double elapsed = bench::measure_ms([&]() {
      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int i = start; i < end; ++i) {
                                   vec.push_back(i);
                                }
                             });

      assert(vec.size() == static_cast<size_t>(total_elements));

      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int i = start; i < end; ++i) {
                                   size_t idx = static_cast<size_t>(i);
                                   int val = vec.get(idx) + 1;
                                   vec.set(idx, val);
                                }
                             });

      bench::parallel_chunks(num_threads, total_elements / 2,
                             [&](int start, int end) {
                                for (int i = start; i < end; ++i) {
                                   vec.pop_back();
                                }
                             });

      vec.clear();
   });

   assert(vec.empty());
   return elapsed;
}

}  // namespace perf