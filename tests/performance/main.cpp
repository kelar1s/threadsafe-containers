#include <iostream>
#include <string>

#include "lockfree_forward_list.hpp"
#include "lockfree_list.hpp"
#include "lockfree_vector.hpp"
#include "mutex_forward_list.hpp"
#include "mutex_list.hpp"
#include "mutex_vector.hpp"
#include "utils/bench_engine.hpp"
#include "workloads/combined/forward_list_bench.hpp"
#include "workloads/combined/list_bench.hpp"
#include "workloads/combined/vector_bench.hpp"
#include "workloads/operations/forward_list_ops.hpp"
#include "workloads/operations/list_ops.hpp"
#include "workloads/operations/vector_ops.hpp"

void run_operations_benchmarks() {
   // --- VECTOR ---
   bench::run_comparison_table(
       "Vector", "push_back",
       perf::ops::vector_push_back<ts::mutex_vector<int>>,
       perf::ops::vector_push_back<ts::lockfree_vector<int>>);

   bench::run_setup_comparison_table<ts::mutex_vector<int>,
                                     ts::lockfree_vector<int>>(
       "Vector", "get", perf::ops::VectorSetup{}, perf::ops::VectorGetRun{});

   bench::run_setup_comparison_table<ts::mutex_vector<int>,
                                     ts::lockfree_vector<int>>(
       "Vector", "set", perf::ops::VectorSetup{}, perf::ops::VectorSetRun{});

   bench::run_comparison_table(
       "Vector", "pop_back", perf::ops::vector_pop_back<ts::mutex_vector<int>>,
       perf::ops::vector_pop_back<ts::lockfree_vector<int>>);

   // --- FORWARD LIST ---
   bench::run_comparison_table(
       "ForwardList", "push_front",
       perf::ops::flist_push_front<ts::mutex_forward_list<int>>,
       perf::ops::flist_push_front<ts::lockfree_forward_list<int>>);

   bench::run_setup_comparison_table<ts::mutex_forward_list<int>,
                                     ts::lockfree_forward_list<int>>(
       "ForwardList", "contains", perf::ops::ForwardListSetup{},
       perf::ops::ForwardListContainsRun{});

   bench::run_comparison_table(
       "ForwardList", "pop_front",
       perf::ops::flist_pop_front<ts::mutex_forward_list<int>>,
       perf::ops::flist_pop_front<ts::lockfree_forward_list<int>>);

   // --- LIST ---
   bench::run_comparison_table(
       "List", "push_front", perf::ops::list_push_front<ts::mutex_list<int>>,
       perf::ops::list_push_front<ts::lockfree_list<int>>);

   bench::run_comparison_table(
       "List", "push_back", perf::ops::list_push_back<ts::mutex_list<int>>,
       perf::ops::list_push_back<ts::lockfree_list<int>>);

   bench::run_setup_comparison_table<ts::mutex_list<int>,
                                     ts::lockfree_list<int>>(
       "List", "contains", perf::ops::ListSetup{},
       perf::ops::ListContainsRun{});

   bench::run_comparison_table(
       "List", "pop_front", perf::ops::list_pop_front<ts::mutex_list<int>>,
       perf::ops::list_pop_front<ts::lockfree_list<int>>);

   bench::run_comparison_table(
       "List", "pop_back", perf::ops::list_pop_back<ts::mutex_list<int>>,
       perf::ops::list_pop_back<ts::lockfree_list<int>>);

   bench::run_comparison_table("List", "remove",
                               perf::ops::list_remove<ts::mutex_list<int>>,
                               perf::ops::list_remove<ts::lockfree_list<int>>);
}

void run_combined_benchmarks() {
   bench::run_comparison_table("Vector", "Combined",
                               perf::vector_workload<ts::mutex_vector<int>>,
                               perf::vector_workload<ts::lockfree_vector<int>>);

   bench::run_comparison_table(
       "ForwardList", "Combined",
       perf::forward_list_workload<ts::mutex_forward_list<int>>,
       perf::forward_list_workload<ts::lockfree_forward_list<int>>);

   bench::run_comparison_table("List", "Combined",
                               perf::list_workload<ts::mutex_list<int>>,
                               perf::list_workload<ts::lockfree_list<int>>);
}

int main(int argc, char* argv[]) {
   std::string mode = (argc > 1) ? argv[1] : "--ops";

   if (mode == "--ops") {
      run_operations_benchmarks();
   } else if (mode == "--combined") {
      run_combined_benchmarks();
   } else if (mode == "--all") {
      run_operations_benchmarks();
      run_combined_benchmarks();
   } else {
      std::cout << "Usage: " << argv[0] << " [--ops | --combined | --all]\n";
      return 1;
   }

   return 0;
}