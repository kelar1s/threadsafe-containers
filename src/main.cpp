#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

#include "threadsafe_vector.hpp"

void test_concurrent_push() {
   ts::vector<int> vec;
   const int num_threads = 8;
   const int ops_per_thread = 10000;

   std::cout << "[Test 1] push_back" << std::endl;

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
   std::cout << "Ожидаемый размер: " << expected_size << std::endl;
   std::cout << "Фактический размер: " << vec.size() << std::endl;

   assert(vec.size() == expected_size);
   std::cout << "[Success]\n" << std::endl;
}

void test_concurrent_read_write() {
   std::cout << "[Test 2] push_back + get" << std::endl;
   ts::vector<int> vec;
   const int num_writers = 4;
   const int num_readers = 4;
   const int ops_per_writer = 5000;

   std::atomic<bool> stop_readers = false;
   std::atomic<int> successful_reads = 0;

   std::vector<std::thread> threads;

   for (int r = 0; r < num_readers; ++r) {
      threads.push_back(std::thread([&vec, &stop_readers, &successful_reads]() {
         while (!stop_readers) {
            size_t current_sz = vec.size();
            if (current_sz > 0) {
               int val;
               if (vec.get(current_sz - 1, val)) {
                  successful_reads++;
               }
            }
         }
      }));
   }

   for (int w = 0; w < num_writers; ++w) {
      threads.push_back(std::thread([&vec, w, ops_per_writer]() {
         for (int i = 0; i < ops_per_writer; ++i) {
            vec.push_back(w * ops_per_writer + i);
         }
      }));
   }

   for (size_t i = num_readers; i < threads.size(); ++i) {
      threads[i].join();
   }

   stop_readers = true;
   for (size_t i = 0; i < num_readers; ++i) {
      threads[i].join();
   }

   std::cout << "Всего элементов записано: " << vec.size() << std::endl;
   std::cout << "Успешных чтений во время записи: " << successful_reads
             << std::endl;

   assert(vec.size() == num_writers * ops_per_writer);
   std::cout << "[Success]\n" << std::endl;
}

void test_concurrent_pop() {
   std::cout << "[Test 3] pop_back" << std::endl;
   ts::vector<int> vec;
   const int init_count = 20000;

   for (int i = 0; i < init_count; ++i) {
      vec.push_back(i);
   }

   const int num_consumers = 4;
   std::atomic<int> total_popped = 0;
   std::vector<std::thread> consumers;

   for (int c = 0; c < num_consumers; ++c) {
      consumers.push_back(std::thread([&vec, &total_popped]() {
         while (true) {
            int val;

            if (!vec.pop_back(val)) {
               break;
            }
            total_popped++;
         }
      }));
   }

   for (auto& c : consumers) {
      c.join();
   }

   std::cout << "Изначально элементов: " << init_count << std::endl;
   std::cout << "Всего извлечено: " << total_popped << std::endl;
   std::cout << "Итоговый размер вектора: " << vec.size() << std::endl;

   assert(total_popped == init_count);
   assert(vec.empty());
   std::cout << "[Success]\n" << std::endl;
}

int main() {
   test_concurrent_push();
   test_concurrent_read_write();
   test_concurrent_pop();

   std::cout << "All tests completed successfully" << std::endl;
   return 0;
}