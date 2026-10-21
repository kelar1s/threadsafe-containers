#include <atomic>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "lockfree_vector.hpp"
#include "mutex_vector.hpp"

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

   for (int i = 0; i < count; ++i) {
      assert(vec[i] == i + 15);
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

template <typename VectorType>
double measure_performance(int num_threads, int total_elements) {
   VectorType vec;
   int ops_per_thread = total_elements / num_threads;

   auto start = std::chrono::high_resolution_clock::now();

   std::vector<std::thread> threads;
   for (int t = 0; t < num_threads; ++t) {
      threads.push_back(std::thread([&vec, ops_per_thread]() {
         for (int i = 0; i < ops_per_thread; ++i) {
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
      threads.push_back(std::thread([&vec, t, ops_per_thread]() {
         size_t base = static_cast<size_t>(t * ops_per_thread);
         for (int i = 0; i < ops_per_thread; ++i) {
            size_t idx = base + i;
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

   auto end = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double, std::milli> elapsed = end - start;
   return elapsed.count();
}

void run_benchmarks() {
   const int total_elements = 600000;
   const std::vector<int> thread_counts = {1, 2, 4, 6};

   std::cout << std::left << std::setw(10) << "Threads " << std::setw(22)
             << "Mutex Vector (ms) " << std::setw(22)
             << "Lock-Free Vector (ms) "
             << "Speedup" << std::endl;
   std::cout << std::string(65, '-') << std::endl;

   for (int threads : thread_counts) {
      double mutex_time =
          measure_performance<ts::mutex_vector<int>>(threads, total_elements);
      double lockfree_time = measure_performance<ts::lockfree_vector<int>>(
          threads, total_elements);
      double speedup = mutex_time / lockfree_time;

      std::cout << std::left << std::setw(10) << threads << std::fixed
                << std::setprecision(2) << std::setw(22) << mutex_time
                << std::setw(22) << lockfree_time << speedup << "x"
                << std::endl;
   }
   std::cout << std::endl;
}

int main() {
   test_mutex_push();
   test_mutex_index_and_set();
   test_mutex_pop();

   test_lockfree_push();
   test_lockfree_index_and_set();
   test_lockfree_pop();

   run_benchmarks();

   return 0;
}
