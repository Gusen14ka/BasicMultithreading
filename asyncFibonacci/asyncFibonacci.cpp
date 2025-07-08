#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <chrono>
#include <future>
#include <ostream>
#include <thread>

constexpr int MAX_N = 93;

// Function with memoization computing Fibonacci number
// Coukld be used with async
int fibonacciMem(int n, std::unordered_map<int, int>& memo){
    if(n <= 1){
        return n;
    }
    if(memo.contains(n)){
        return memo[n];
    }
    memo[n] = fibonacciMem(n - 1, memo) + fibonacciMem(n - 2, memo);
    return memo[n];
}

// Recursive Fibonacci function
// Could be used with async
int fibonacciRec(int n){
    if (n <= 1){
        return n;
    }
    return fibonacciRec(n - 1) + fibonacciRec(n - 2);
}

// Atomic Fibonacci function using atomic cache
// Cache is uint64_t array
uint64_t fibonacciAtomic(int n, auto& cache){
    auto val = cache[n].load(std::memory_order_relaxed);
    if(val != UINT64_MAX){
        return val;
    }
    auto a = fibonacciAtomic(n - 1, cache);
    auto b = fibonacciAtomic(n - 2, cache);
    auto sum = a + b;
    cache[n].compare_exchange_strong(
        val,
        sum,
        std::memory_order_release,
        std::memory_order_relaxed
    );
    return cache[n].load(std::memory_order_acquire);

}

// Initialize atomic cache
void initCache(auto& cache){
    for(int i = 2; i < MAX_N; i++){
        cache[i].store(UINT64_MAX, std::memory_order_relaxed);
    }
    cache[0].store(0, std::memory_order_relaxed);
    cache[1].store(1, std::memory_order_relaxed);
}

// Non-atomic/Basic Fibonacci function using a simple array cache
uint64_t fibonacciNotAtomic(int n, auto& cache){
    auto val = cache[n];
    if(val != UINT64_MAX){
        return val;
    }
    auto a = fibonacciNotAtomic(n - 1, cache);
    auto b = fibonacciNotAtomic(n - 2, cache);
    auto sum = a + b;
    cache[n] = sum;
    return sum;

}

// Initialize non-atomic cache
// Cache is uint64_t array
void initCacheNotAtomic(auto& cache){
    for(int i = 2; i < MAX_N; i++){
        cache[i] = UINT64_MAX;
    }
    cache[0] = static_cast<uint64_t>(0);
    cache[1] = static_cast<uint64_t>(0);
}

// Thread-safe Fibonacci function using shared_mutex
int fibonacciThred(int n, std::shared_mutex& mut, auto& memo){
    {
        std::shared_lock lk(mut);
        if(memo.contains(n)){
            return memo[n];
        }
    }
    auto a = fibonacciThred(n - 1, std::ref(mut), memo);
    auto b = fibonacciThred(n - 2, std::ref(mut), memo);
    auto sum = a + b;
    {
        std::unique_lock lk(mut);
        if(!memo.contains(n)){
            memo[n] = sum;
        }
        return memo[n];
    }
}

// Thread-safe Fibonacci function using atomic shared_ptr
// Uses atomic shared_ptr to manage the cache
int fibonacciSharedPtrAtomic(int n, auto& cachePtr){
    auto snapshot = cachePtr.load(std::memory_order_acquire);
    if(snapshot->contains(n)){
        return snapshot->at(n);
    }
    auto a = fibonacciSharedPtrAtomic(n - 1, cachePtr);
    auto b = fibonacciSharedPtrAtomic(n - 2, cachePtr);
    auto sum = a + b;
    auto newMap = std::make_shared<std::unordered_map<int, int>>(*snapshot);
    (*newMap)[n] = sum;
    cachePtr.compare_exchange_strong(
        snapshot,
        newMap,
        std::memory_order_release,
        std::memory_order_relaxed
    );
    return sum;
}

int main(){
    {
        std::unordered_map<int, int> memo;
        memo[0] = 0;
        memo[1] = 1;
        auto start = std::chrono::high_resolution_clock::now();
        auto f1 = std::async(std::launch::async, fibonacciMem, 40, ref(memo));
        auto f2 = std::async(std::launch::async, fibonacciMem, 41, ref(memo));
        auto f3 = std::async(std::launch::async, fibonacciMem, 42, ref(memo));

        auto f1_res = f1.get();
        auto f2_res = f2.get();
        auto f3_res = f3.get();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time for async fibonacciMem " << end - start << std::endl;
    }
    
    {
        std::unordered_map<int, int> memo;
        memo[0] = 0;
        memo[1] = 1;
        auto start = std::chrono::high_resolution_clock::now();
        auto f1 = fibonacciMem(40, memo);
        auto f2 = fibonacciMem(41, memo);
        auto f3 = fibonacciMem(42, memo);
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time for NOT async fibonacciMem " << end - start << std::endl;
    }

    {
        auto start = std::chrono::high_resolution_clock::now();
        auto f1 = std::async(std::launch::async, fibonacciRec, 40);
        auto f2 = std::async(std::launch::async, fibonacciRec, 41);
        auto f3 = std::async(std::launch::async, fibonacciRec, 42);

        auto f1_res = f1.get();
        auto f2_res = f2.get();
        auto f3_res = f3.get();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time for async fibonacciRec " << end - start << std::endl;
    }

    {
        auto start = std::chrono::high_resolution_clock::now();
        auto f1 = fibonacciRec(40);
        auto f2 = fibonacciRec(41);
        auto f3 = fibonacciRec(42);
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time for NOT async fibonacciRec " << end - start << std::endl;
    }

    {
        std::atomic<uint64_t> cache[MAX_N];
        auto start = std::chrono::high_resolution_clock::now();
        initCache(cache);
        auto f1 = fibonacciAtomic(40, cache);
        auto f2 = fibonacciAtomic(41, cache);
        auto f3 = fibonacciAtomic(42, cache);
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time for atomic fibonacci " << end - start << std::endl;
    }

    {
        uint64_t cache[MAX_N];
        auto start = std::chrono::high_resolution_clock::now();
        initCacheNotAtomic(cache);
        auto f1 = fibonacciNotAtomic(40, cache);
        auto f2 = fibonacciNotAtomic(41, cache);
        auto f3 = fibonacciNotAtomic(42, cache);
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time for NOT atomic fibonacci " << end - start << std::endl;
    }

    {
        std::unordered_map<int, int> memo;
        auto start = std::chrono::high_resolution_clock::now();
        memo[0] = 0;
        memo[1] = 1;
        auto f1 = 0, f2 = 0, f3 = 0;
        std::shared_mutex mut;
        std::thread t1([&]{f1 = fibonacciThred(40, std::ref(mut), memo);});
        std::thread t2([&]{f1 = fibonacciThred(41, std::ref(mut), memo);}); 
        std::thread t3([&]{f1 = fibonacciThred(42, std::ref(mut), memo);});
        t1.join();
        t2.join();
        t3.join();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time for thread shared_mutex fibonacci " << end - start << std::endl;
    }
    {
        
        using MapPtr = std::shared_ptr<std::unordered_map<int, int>>;
        MapPtr memo = std::make_shared<std::unordered_map<int, int>>();
        std::atomic<MapPtr> cachePtr;
        auto start = std::chrono::high_resolution_clock::now();
        (*memo)[0] = 0;
        (*memo)[1] = 1;
        cachePtr.store(memo, std::memory_order_release);
        auto f1 = 0, f2 = 0, f3 = 0;
        std::thread t1([&]{f1 = fibonacciSharedPtrAtomic(40, cachePtr);});
        std::thread t2([&]{f2 = fibonacciSharedPtrAtomic(41, cachePtr);});
        std::thread t3([&]{f3 = fibonacciSharedPtrAtomic(42, cachePtr);});
        t1.join();
        t2.join();
        t3.join();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time for thread atomic shared_ptr fibonacci " << end - start << std::endl;
    }
}

/*
Possible output
Time for async fibonacciMem 928400ns
Time for NOT async fibonacciMem 27200ns
Time for async fibonacciRec 1997540300ns
Time for NOT async fibonacciRec 3962125000ns
Time for atomic fibonacci 3800ns
Time for NOT atomic fibonacci 1300ns
Time for thread shared_mutex fibonacci 3684800ns
Time for thread atomic shared_ptr fibonacci 2933600ns
*/