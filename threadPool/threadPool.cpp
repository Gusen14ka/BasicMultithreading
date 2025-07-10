#include "threadPool.hpp" 
#include <iostream>
#include <vector>
#include <chrono>
#include <future>


int main() {
    {
        auto start = std::chrono::high_resolution_clock::now();
        ThreadPool_jthread pool(4);

        std::vector<std::future<int>> results;
        results.reserve(8);

        // mutex fpr cout
        std::mutex coutMtx;
        // Launch 8 tasks
        for (int i = 0; i < 8; ++i) {
            results.emplace_back(
                pool.enqueue([i, &coutMtx]{
                    // Simulate work
                    std::this_thread::sleep_for(std::chrono::milliseconds(100 * (i % 3 + 1)));

                    // Locate output in one string
                    std::ostringstream ss;
                    ss << "Task " << i
                    << " executed in thread " << std::this_thread::get_id()
                    << "\n";

                    // Safety print
                    {
                        std::lock_guard lk(coutMtx);
                        std::cout << ss.str();
                    }

                    return i * i;
                })
            );
        }

        // Get results
        for (auto &fut : results) {
            int val = fut.get();
            std::lock_guard lk(coutMtx);
            std::cout << "Result: " << val << "\n";
        }
        pool.shoutdown();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time for jthread pool = " << end - start << std::endl;
    }
    
    {
        auto start = std::chrono::high_resolution_clock::now();
        ThreadPool_thread pool(4);

        std::vector<std::future<int>> results;
        results.reserve(8);

        // mutex fpr cout
        std::mutex coutMtx;
        // Launch 8 tasks
        for (int i = 0; i < 8; ++i) {
            results.emplace_back(
                pool.enqueue([i, &coutMtx]{
                    // Simulate work
                    std::this_thread::sleep_for(std::chrono::milliseconds(100 * (i % 3 + 1)));

                    // Locate output in one string
                    std::ostringstream ss;
                    ss << "Task " << i
                    << " executed in thread " << std::this_thread::get_id()
                    << "\n";

                    // Safety print
                    {
                        std::lock_guard lk(coutMtx);
                        std::cout << ss.str();
                    }

                    return i * i;
                })
            );
        }

        // Get results
        for (auto &fut : results) {
            int val = fut.get();
            std::lock_guard lk(coutMtx);
            std::cout << "Result: " << val << "\n";
        }
        pool.shoutdown();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time for thread pool = " << end - start << std::endl;
    }
}