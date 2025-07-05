#include <atomic>
#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <chrono>

// Global counters
long long g_counter1 = 0;
long long g_counter2 = 0;
std::atomic<long long> atom_counter = 0;
constexpr long long N = 1000000000;

// Function to join threads
void joinThreads(auto& vec){
    for(auto& t : vec){
        t.join();
    }
}

int main(){

    std::mutex mut;

    // Without synchronization - wrong result
    auto f_race = [](){
        for(int i = 0; i < N; i++){
            g_counter1++;
        }
    };

    // With mutex - correct result, but slow
    auto f_mut_race = [&mut](){
        for(int i = 0; i < N; i++){
            {
                std::lock_guard<std::mutex> lk(mut);
                g_counter2++;
            }
        }
    };

    // With atomic - correct result, fast
    auto f_atomic_race = [](){
        for(int i = 0; i < N; i++){
            atom_counter++;
        }
    };

    // With atomic relaxed - correct result, very fast, but no memory ordering guarantees
    auto f_atomic_relaxed_race = [](){
        for(int i = 0; i < N; i++){
            atom_counter.fetch_add(1, std::memory_order_relaxed);
        }
    };
    
    
    std::vector<std::thread> threads;

    // Prepare threads for experiment without synchronization
    for(int i = 0; i < 4; i++){
        threads.push_back(std::thread(f_race));
    }
    auto start = std::chrono::high_resolution_clock::now();
    joinThreads(threads);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Result counter after race = " << g_counter1 << std::endl;
    std::chrono::duration<double, std::milli> duration_ms = end - start;
    std::cout << "Taken time by race: " << duration_ms.count() << " ms\n";
    threads.clear();

    // Prepare threads for experiment with mutex
    for(int i = 0; i < 4; i++){
        threads.push_back(std::thread(f_mut_race));
    }
    start = std::chrono::high_resolution_clock::now();
    joinThreads(threads);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Result counter after mutex race = " << g_counter2 << std::endl;
    duration_ms = end - start;
    std::cout << "Taken time by race: " << duration_ms.count() << " ms\n";
    threads.clear();

    // Prepare threads for experiment with atomic
    for(int i = 0; i < 4; i++){
        threads.push_back(std::thread(f_atomic_race));
    }
    start = std::chrono::high_resolution_clock::now();
    joinThreads(threads);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Result counter after atomic race = " << atom_counter << std::endl;
    duration_ms = end - start;
    std::cout << "Taken time by race: " << duration_ms.count() << " ms\n";
    threads.clear();

    atom_counter = 0;

    // Prepare threads for experiment with atomic relaxed
    for(int i = 0; i < 4; i++){
        threads.push_back(std::thread(f_atomic_relaxed_race));
    }
    start = std::chrono::high_resolution_clock::now();
    joinThreads(threads);
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Result counter after atomic relaxed race = " << atom_counter << std::endl;
    duration_ms = end - start;
    std::cout << "Taken time by race: " << duration_ms.count() << " ms\n";

    // Final output:
    /*
    Result counter after race = 1066928920
    Taken time by race: 10250.8 ms
    Result counter after mutex race = 4000000000
    Taken time by race: 207102 ms
    Result counter after atomic race = 4000000000
    Taken time by race: 148295 ms
    Result counter after atomic relaxed race = 4000000000
    Taken time by race: 107277 ms
    */
}