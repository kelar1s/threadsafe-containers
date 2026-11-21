#pragma once

#include <cassert>

#include "../../utils/bench_engine.hpp"

namespace perf::ops {

template <typename ListType>
double flist_push_front(int num_threads, int total_elements) {
   ListType flist;

   double elapsed = bench::measure_ms([&]() {
      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int val = start; val < end; ++val) {
                                   flist.push_front(val);
                                }
                             });
   });

   assert(flist.size() == static_cast<size_t>(total_elements));
   return elapsed;
}

struct ForwardListSetup {
   template <typename ListType>
   void operator()(ListType& flist, int total_elements) const {
      for (int i = 0; i < total_elements; ++i) {
         flist.push_front(i);
      }
   }
};

struct ForwardListContainsRun {
   template <typename ListType>
   double operator()(const ListType& flist, int num_threads,
                     int total_elements) const {
      constexpr int kTotalSearches = 60;
      int step = total_elements / kTotalSearches;

      return bench::measure_ms([&]() {
         bench::parallel_chunks(num_threads, kTotalSearches,
                                [&](int start, int end) {
                                   for (int i = start; i < end; ++i) {
                                      bool found = flist.contains(i * step);
                                      assert(found);
                                   }
                                });
      });
   }
};

template <typename ListType>
double flist_pop_front(int num_threads, int total_elements) {
   ListType flist;
   for (int i = 0; i < total_elements; ++i) {
      flist.push_front(i);
   }

   double elapsed = bench::measure_ms([&]() {
      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int i = start; i < end; ++i) {
                                   flist.pop_front();
                                }
                             });
   });

   assert(flist.empty());
   return elapsed;
}

}  // namespace perf::ops