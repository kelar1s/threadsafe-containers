#pragma once

#include <cassert>

#include "../../utils/bench_engine.hpp"

namespace perf {

template <typename ListType>
double list_workload(int num_threads, int total_elements) {
   ListType lst;
   int searches_per_thread = 144 / num_threads;

   double elapsed = bench::measure_ms([&]() {
      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int val = start; val < end; ++val) {
                                   if (val % 2 == 0)
                                      lst.push_front(val);
                                   else
                                      lst.push_back(val);
                                }
                             });

      assert(lst.size() == static_cast<size_t>(total_elements));

      bench::parallel_chunks(num_threads, total_elements, [&](int, int end) {
         for (int i = 0; i < searches_per_thread; ++i) {
            bool found = lst.contains(end - 1 - i);
            assert(found);
         }
      });

      bench::parallel_chunks(num_threads, total_elements / 2,
                             [&](int start, int end) {
                                for (int i = start; i < end; ++i) {
                                   if (i % 2 == 0)
                                      lst.pop_front();
                                   else
                                      lst.pop_back();
                                }
                             });

      lst.clear();
   });

   assert(lst.empty());
   return elapsed;
}

}  // namespace perf