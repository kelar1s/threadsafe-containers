#include <atomic>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "lockfree_forward_list.hpp"
#include "lockfree_vector.hpp"
#include "mutex_vector.hpp"

// ########################### Mutex Vector

void test_mutex_push() {
   ts::mutex_vector<int> vec;
   const int num_threads = 8;
   const int ops_per_thread = 10000;

   std::cout << "[Mutex Vector] TEST: push_back" << std::endl;
   std::vector<std::thread> workers;
   for (int t = 0; t < num_threads; ++t) {
      workers.push_back(std::thread([&vec, t, ops_per_thread]() {
         for (int i = 0; i < ops_per_thread; ++i) {
            vec.push_back(t * ops_per_thread + i);
         }
      }));
   }

   for (auto& w : workers) {
      w.join();
   }

   const size_t expected_size = num_threads * ops_per_thread;
   assert(vec.size() == expected_size);
   std::cout << "    [SUCCES] size: " << vec.size() << "\n" << std::endl;
}

void test_mutex_index_and_set() {
   std::cout << "[Mutex Vector] TEST: [], set, get" << std::endl;
   ts::mutex_vector<int> vec;
   const int count = 5000;

   for (int i = 0; i < count; ++i) {
      vec.push_back(i);
   }

   const int num_threads = 4;
   std::vector<std::thread> threads;
   for (int t = 0; t < num_threads; ++t) {
      threads.push_back(std::thread([&vec, t, count, num_threads]() {
         int chunk = count / num_threads;
         int start = t * chunk;
         int end = start + chunk;
         for (int i = start; i < end; ++i) {
            vec[i] += 10;
            vec.set(i, vec.get(i) + 5);
         }
      }));
   }

   for (auto& th : threads) {
      th.join();
   }

   const auto& const_vec = vec;
   for (int i = 0; i < count; ++i) {
      assert(const_vec[i] == i + 15);
   }
   std::cout << "    [SUCCESS]\n" << std::endl;
}

void test_mutex_pop() {
   ts::mutex_vector<int> vec;
   const int init_count = 20000;
   for (int i = 0; i < init_count; ++i) {
      vec.push_back(i);
   }

   std::cout << "[Mutex Vector] TEST: pop_back" << std::endl;
   const int num_threads = 8;
   std::atomic<int> total_popped = 0;
   std::vector<std::thread> threads;

   for (int t = 0; t < num_threads; ++t) {
      threads.push_back(std::thread([&vec, &total_popped]() {
         while (true) {
            if (vec.empty()) break;
            vec.pop_back();
            total_popped++;
         }
      }));
   }

   for (auto& t : threads) {
      t.join();
   }

   assert(vec.empty());
   std::cout << "    [SUCCES] total popped: " << total_popped << "\n"
             << std::endl;
}

void test_mutex_size_empty() {
   std::cout << "[Mutex Vector] TEST: concurrent size and empty" << std::endl;
   ts::mutex_vector<int> vec;
   const size_t max_ops = 15000;
   std::atomic<bool> writer_done{false};
   std::atomic<size_t> checks_count{0};

   std::thread writer([&vec, max_ops, &writer_done]() {
      for (size_t i = 0; i < max_ops; ++i) {
         vec.push_back(static_cast<int>(i));
         if (i % 3 == 0) {
            vec.pop_back();
         }
      }
      writer_done.store(true);
   });

   std::thread reader([&vec, max_ops, &writer_done, &checks_count]() {
      while (!writer_done.load()) {
         size_t sz = vec.size();
         bool is_empty = vec.empty();
         assert(sz <= max_ops);
         if (is_empty) {
            assert(vec.size() <= max_ops);
         }
         checks_count++;
      }
   });

   writer.join();
   reader.join();

   const size_t expected_final_size = max_ops - (max_ops / 3);
   assert(vec.size() == expected_final_size);
   assert(!vec.empty());
   std::cout << "    [SUCCESS] final size: " << vec.size()
             << ", concurrent checks: " << checks_count << "\n"
             << std::endl;
}

// ########################### Lockfree Vector

void test_lockfree_push() {
   ts::lockfree_vector<int> vec;
   const int num_threads = 8;
   const int ops_per_thread = 10000;

   std::cout << "[LockFree Vector] TEST: push_back" << std::endl;
   std::vector<std::thread> workers;
   for (int t = 0; t < num_threads; ++t) {
      workers.push_back(std::thread([&vec, t, ops_per_thread]() {
         for (int i = 0; i < ops_per_thread; ++i) {
            vec.push_back(t * ops_per_thread + i);
         }
      }));
   }

   for (auto& w : workers) {
      w.join();
   }

   const size_t expected_size = num_threads * ops_per_thread;
   assert(vec.size() == expected_size);
   std::cout << "    [SUCCESS] size: " << vec.size() << "\n" << std::endl;
}

void test_lockfree_index_and_set() {
   std::cout << "[LockFree Vector] TEST: [], set, get" << std::endl;
   ts::lockfree_vector<int> vec;
   const int count = 5000;

   for (int i = 0; i < count; ++i) {
      vec.push_back(i);
   }

   const int num_threads = 4;
   std::vector<std::thread> threads;
   for (int t = 0; t < num_threads; ++t) {
      threads.push_back(std::thread([&vec, t, count, num_threads]() {
         int chunk = count / num_threads;
         int start = t * chunk;
         int end = start + chunk;
         for (int i = start; i < end; ++i) {
            vec[i] += 10;
            vec.set(i, vec.get(i) + 5);
         }
      }));
   }

   for (auto& th : threads) {
      th.join();
   }

   for (int i = 0; i < count; ++i) {
      assert(vec[i] == i + 15);
   }
   std::cout << "    [SUCCESS] \n" << std::endl;
}

void test_lockfree_pop() {
   ts::lockfree_vector<int> vec;
   const int init_count = 20000;
   for (int i = 0; i < init_count; ++i) {
      vec.push_back(i);
   }

   std::cout << "[LockFree Vector] ТEST: pop_back" << std::endl;
   const int num_threads = 4;
   std::atomic<int> total_popped = 0;
   std::vector<std::thread> threads;

   for (int t = 0; t < num_threads; ++t) {
      threads.push_back(std::thread([&vec, &total_popped]() {
         while (true) {
            if (vec.empty()) break;
            vec.pop_back();
            total_popped++;
         }
      }));
   }

   for (auto& t : threads) {
      t.join();
   }

   assert(vec.empty());
   std::cout << "    [SUCCESS] total popped: " << total_popped << "\n"
             << std::endl;
}

void test_lockfree_size_empty() {
   std::cout << "[LockFree Vector] TEST: size and empty" << std::endl;
   ts::lockfree_vector<int> vec;
   const size_t max_ops = 15000;
   std::atomic<bool> writer_done{false};
   std::atomic<size_t> checks_count{0};

   std::thread writer([&vec, max_ops, &writer_done]() {
      for (size_t i = 0; i < max_ops; ++i) {
         vec.push_back(static_cast<int>(i));
         if (i % 3 == 0) {
            vec.pop_back();
         }
      }
      writer_done.store(true);
   });

   std::thread reader([&vec, max_ops, &writer_done, &checks_count]() {
      while (!writer_done.load()) {
         size_t sz = vec.size();
         bool is_empty = vec.empty();
         assert(sz <= max_ops);
         if (is_empty) {
            assert(vec.size() <= max_ops);
         }
         checks_count++;
      }
   });

   writer.join();
   reader.join();

   const size_t expected_final_size = max_ops - (max_ops / 3);
   assert(vec.size() == expected_final_size);
   assert(!vec.empty());
   std::cout << "    [SUCCESS] final size: " << vec.size()
             << ", concurrent checks: " << checks_count << "\n"
             << std::endl;
}

// ########################### Lockfree Forward List

void test_lockfree_forward_list_push_and_contains() {
   ts::lockfree_forward_list<int> flist;
   const int num_threads = 8;
   const int ops_per_thread = 200;

   std::cout << "[LockFree ForwardList] TEST: push_front and contains"
             << std::endl;
   std::vector<std::thread> workers;
   for (int t = 0; t < num_threads; ++t) {
      workers.push_back(std::thread([&flist, t, ops_per_thread]() {
         for (int i = 0; i < ops_per_thread; ++i) {
            flist.push_front(t * ops_per_thread + i);
         }
      }));
   }

   for (auto& w : workers) {
      w.join();
   }

   const size_t expected_size = num_threads * ops_per_thread;
   assert(flist.size() == expected_size);

   workers.clear();

   for (int t = 0; t < num_threads; ++t) {
      workers.push_back(std::thread([&flist, t, ops_per_thread]() {
         for (int i = 0; i < ops_per_thread; ++i) {
            assert(flist.contains(t * ops_per_thread + i));
         }
      }));
   }

   for (auto& w : workers) {
      w.join();
   }

   assert(!flist.contains(-1));
   assert(!flist.contains(static_cast<int>(expected_size + 100)));

   std::cout << "    [SUCCESS] size: " << flist.size() << "\n" << std::endl;
}

void test_lockfree_forward_list_pop() {
   ts::lockfree_forward_list<int> flist;
   const int init_count = 20000;
   for (int i = 0; i < init_count; ++i) {
      flist.push_front(i);
   }

   std::cout << "[LockFree ForwardList] TEST: pop_front" << std::endl;
   const int num_threads = 8;
   std::atomic<int> total_popped = 0;
   std::vector<std::thread> threads;

   for (int t = 0; t < num_threads; ++t) {
      threads.push_back(std::thread([&flist, &total_popped]() {
         while (true) {
            if (flist.empty()) break;
            flist.pop_front();
            total_popped++;
         }
      }));
   }

   for (auto& t : threads) {
      t.join();
   }

   assert(flist.empty());
   assert(!flist.contains(0));
   std::cout << "    [SUCCESS] total popped: " << total_popped << "\n"
             << std::endl;
}

void test_lockfree_forward_list_size_empty() {
   std::cout << "[LockFree ForwardList] TEST: concurrent size and empty"
             << std::endl;
   ts::lockfree_forward_list<int> flist;
   const size_t max_ops = 15000;
   std::atomic<bool> writer_done{false};
   std::atomic<size_t> checks_count{0};

   std::thread writer([&flist, max_ops, &writer_done]() {
      for (size_t i = 0; i < max_ops; ++i) {
         flist.push_front(static_cast<int>(i));
         if (i % 3 == 0) {
            flist.pop_front();
         }
      }
      writer_done.store(true);
   });

   std::thread reader([&flist, max_ops, &writer_done, &checks_count]() {
      while (!writer_done.load()) {
         size_t sz = flist.size();
         assert(sz <= max_ops);
         checks_count++;
      }
   });

   writer.join();
   reader.join();

   const size_t expected_final_size = max_ops - (max_ops / 3);
   assert(flist.size() == expected_final_size);
   assert(!flist.empty());
   std::cout << "    [SUCCESS] final size: " << flist.size()
             << ", concurrent checks: " << checks_count << "\n"
             << std::endl;
}

void test_lockfree_forward_list_clear() {
   std::cout << "[LockFree ForwardList] TEST: clear()" << std::endl;
   ts::lockfree_forward_list<int> flist;

   for (int i = 0; i < 5000; ++i) {
      flist.push_front(i);
   }
   assert(flist.size() == 5000);
   assert(!flist.empty());

   flist.clear();
   assert(flist.empty());
   assert(flist.size() == 0);
   assert(!flist.contains(100));

   flist.push_front(42);
   assert(!flist.empty());
   assert(flist.size() == 1);
   assert(flist.contains(42));
   std::cout << "    [SUCCESS]\n" << std::endl;
}

template <typename VectorType>
double measure_single_run(int num_threads, int total_elements) {
   VectorType vec;
   int chunk = total_elements / num_threads;

   auto start_time = std::chrono::high_resolution_clock::now();

   std::vector<std::thread> threads;
   for (int t = 0; t < num_threads; ++t) {
      int start_idx = t * chunk;
      int end_idx = (t == num_threads - 1) ? total_elements : (t + 1) * chunk;
      int ops = end_idx - start_idx;

      threads.push_back(std::thread([&vec, ops]() {
         for (int i = 0; i < ops; ++i) {
            vec.push_back(0);
         }
      }));
   }
   for (auto& th : threads) {
      th.join();
   }

   assert(vec.size() == static_cast<size_t>(total_elements));

   threads.clear();
   for (int t = 0; t < num_threads; ++t) {
      size_t start_idx = static_cast<size_t>(t * chunk);
      size_t end_idx = (t == num_threads - 1)
                           ? static_cast<size_t>(total_elements)
                           : static_cast<size_t>((t + 1) * chunk);

      threads.push_back(std::thread([&vec, start_idx, end_idx]() {
         for (size_t idx = start_idx; idx < end_idx; ++idx) {
            vec[idx] = static_cast<int>(idx + 1);
            vec.set(idx, vec.get(idx) + 1);
         }
      }));
   }
   for (auto& th : threads) {
      th.join();
   }

   for (size_t i = 0; i < static_cast<size_t>(total_elements); ++i) {
      int expected = static_cast<int>(i + 2);
      assert(vec.get(i) == expected);
   }

   vec.clear();
   assert(vec.empty());

   auto end_time = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
   return elapsed.count();
}

template <typename VectorType>
double measure_performance(int num_threads, int total_elements,
                           int num_runs = 5) {
   std::vector<double> times;

   for (int i = 0; i < num_runs; ++i) {
      times.push_back(
          measure_single_run<VectorType>(num_threads, total_elements));
   }

   std::sort(times.begin(), times.end());

   double sum = 0.0;
   for (int i = 1; i < num_runs - 1; ++i) {
      sum += times[i];
   }
   return sum / static_cast<double>(num_runs - 2);
}

void run_benchmarks() {
   const std::vector<int> element_counts = {100000, 500000, 1000000};
   const std::vector<int> thread_counts = {1, 2, 4, 6};
   const int num_runs = 5;

   std::cout << std::endl << std::endl;

   for (int total_elements : element_counts) {
      std::cout << "Benchmark: " << total_elements << " elements (" << num_runs
                << " runs)" << std::endl;
      std::cout << std::left << std::setw(10) << "Threads " << std::setw(22)
                << "Mutex Vector (ms) " << std::setw(22)
                << "Lock-Free Vector (ms) " << "Speedup" << std::endl;
      std::cout << std::string(65, '-') << std::endl;

      for (int threads : thread_counts) {
         double mutex_time = measure_performance<ts::mutex_vector<int>>(
             threads, total_elements, num_runs);
         double lockfree_time = measure_performance<ts::lockfree_vector<int>>(
             threads, total_elements, num_runs);
         double speedup = mutex_time / lockfree_time;

         std::cout << std::left << std::setw(10) << threads << std::fixed
                   << std::setprecision(2) << std::setw(22) << mutex_time
                   << std::setw(22) << lockfree_time << speedup << "x"
                   << std::endl;
      }
      std::cout << std::endl << std::endl;
   }
}

int main() {
   test_mutex_push();
   test_mutex_index_and_set();
   test_mutex_pop();
   test_mutex_size_empty();

   test_lockfree_push();
   test_lockfree_index_and_set();
   test_lockfree_pop();
   test_lockfree_size_empty();

   test_lockfree_forward_list_push_and_contains();
   test_lockfree_forward_list_pop();
   test_lockfree_forward_list_size_empty();
   test_lockfree_forward_list_clear();

   run_benchmarks();

   return 0;
}
