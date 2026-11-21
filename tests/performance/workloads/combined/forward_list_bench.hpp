#pragma once

#include <cassert>

#include "../../utils/bench_engine.hpp"

namespace perf {

template <typename ListType>
double forward_list_workload(int num_threads, int total_elements) {
   ListType flist;
   int searches_per_thread = 144 / num_threads;

   double elapsed = bench::measure_ms([&]() {
      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int val = start; val < end; ++val) {
                                   flist.push_front(val);
                                }
                             });

      assert(flist.size() == static_cast<size_t>(total_elements));

      bench::parallel_chunks(num_threads, total_elements, [&](int, int end) {
         for (int i = 0; i < searches_per_thread; ++i) {
            bool found = flist.contains(end - 1 - i);
            assert(found);
         }
      });

      bench::parallel_chunks(num_threads, total_elements / 2,
                             [&](int start, int end) {
                                for (int i = start; i < end; ++i) {
                                   flist.pop_front();
                                }
                             });

      flist.clear();
   });

   assert(flist.empty());
   return elapsed;
}

}  // namespace perf