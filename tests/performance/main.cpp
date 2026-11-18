#include "bench_engine.hpp"
#include "lockfree_forward_list.hpp"
#include "lockfree_list.hpp"
#include "lockfree_vector.hpp"
#include "mutex_forward_list.hpp"
#include "mutex_list.hpp"
#include "mutex_vector.hpp"
#include "workloads/forward_list_bench.hpp"
#include "workloads/list_bench.hpp"
#include "workloads/vector_bench.hpp"

int main() {
  bench::run_comparison_table("Vector",
                              perf::vector_workload<ts::mutex_vector<int>>,
                              perf::vector_workload<ts::lockfree_vector<int>>);

  bench::run_comparison_table(
      "ForwardList", perf::forward_list_workload<ts::mutex_forward_list<int>>,
      perf::forward_list_workload<ts::lockfree_forward_list<int>>);

  bench::run_comparison_table("List", perf::list_workload<ts::mutex_list<int>>,
                              perf::list_workload<ts::lockfree_list<int>>);
  return 0;
}
