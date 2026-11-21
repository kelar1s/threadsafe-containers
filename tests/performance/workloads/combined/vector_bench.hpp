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
                                   vec.push_back(0);
                                }
                             });

      assert(vec.size() == static_cast<size_t>(total_elements));

      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int i = start; i < end; ++i) {
                                   auto idx = static_cast<size_t>(i);
                                   vec[idx] = i + 1;
                                   vec.set(idx, vec.get(idx) + 1);
                                }
                             });
   });

   for (int i = 0; i < total_elements; ++i) {
      assert(vec.get(static_cast<size_t>(i)) == i + 2);
   }

   vec.clear();
   assert(vec.empty());
   return elapsed;
}

}  // namespace perf