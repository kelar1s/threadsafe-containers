#pragma once

#include <cassert>

#include "../../utils/bench_engine.hpp"

namespace perf::ops {

template <typename ListType>
double list_push_front(int num_threads, int total_elements) {
   ListType lst;

   double elapsed = bench::measure_ms([&]() {
      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int val = start; val < end; ++val) {
                                   lst.push_front(val);
                                }
                             });
   });

   assert(lst.size() == static_cast<size_t>(total_elements));
   return elapsed;
}

template <typename ListType>
double list_push_back(int num_threads, int total_elements) {
   ListType lst;

   double elapsed = bench::measure_ms([&]() {
      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int val = start; val < end; ++val) {
                                   lst.push_back(val);
                                }
                             });
   });

   assert(lst.size() == static_cast<size_t>(total_elements));
   return elapsed;
}

struct ListSetup {
   template <typename ListType>
   void operator()(ListType& lst, int total_elements) const {
      for (int i = 0; i < total_elements; ++i) {
         lst.push_back(i);
      }
   }
};

struct ListContainsRun {
   template <typename ListType>
   double operator()(const ListType& lst, int num_threads,
                     int total_elements) const {
      constexpr int kTotalSearches = 60;
      int step = total_elements / kTotalSearches;

      return bench::measure_ms([&]() {
         bench::parallel_chunks(num_threads, kTotalSearches,
                                [&](int start, int end) {
                                   for (int i = start; i < end; ++i) {
                                      bool found = lst.contains(i * step);
                                      assert(found);
                                   }
                                });
      });
   }
};

template <typename ListType>
double list_pop_front(int num_threads, int total_elements) {
   ListType lst;
   for (int i = 0; i < total_elements; ++i) {
      lst.push_back(i);
   }

   double elapsed = bench::measure_ms([&]() {
      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int i = start; i < end; ++i) {
                                   lst.pop_front();
                                }
                             });
   });

   assert(lst.empty());
   return elapsed;
}

template <typename ListType>
double list_pop_back(int num_threads, int total_elements) {
   ListType lst;
   for (int i = 0; i < total_elements; ++i) {
      lst.push_back(i);
   }

   double elapsed = bench::measure_ms([&]() {
      bench::parallel_chunks(num_threads, total_elements,
                             [&](int start, int end) {
                                for (int i = start; i < end; ++i) {
                                   lst.pop_back();
                                }
                             });
   });

   assert(lst.empty());
   return elapsed;
}

template <typename ListType>
double list_remove(int num_threads, int total_elements) {
   ListType lst;
   for (int i = 0; i < total_elements; ++i) {
      lst.push_back(i);
   }

   constexpr int kTotalRemovals = 60;
   int step = total_elements / kTotalRemovals;

   double elapsed = bench::measure_ms([&]() {
      bench::parallel_chunks(num_threads, kTotalRemovals,
                             [&](int start, int end) {
                                for (int i = start; i < end; ++i) {
                                   lst.remove(i * step);
                                }
                             });
   });

   assert(lst.size() == static_cast<size_t>(total_elements - kTotalRemovals));
   return elapsed;
}

}  // namespace perf::ops