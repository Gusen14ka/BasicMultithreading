# Fibonacci Performance Experiments

This project implements and benchmarks several approaches to computing Fibonacci numbers in C++:

1. **Simple Recursion** (`fibonacciRec`)  
2. **Memoized Recursion** (`fibonacciMem`) using `std::unordered_map`  
3. **Asynchronous Recursion** using `std::async`  
4. **Atomic Array-based Memoization** (`fibonacciAtomic`) using `std::atomic<uint64_t>[MAX_N]`  
5. **Non-atomic Array-based Memoization** (`fibonacciNotAtomic`) using plain `uint64_t[MAX_N]`  
6. **Thread-safe Memoization** (`fibonacciThred`) using `std::shared_mutex` + `std::unordered_map`  
7. **Copy-on-Write Memoization** (`fibonacciSharedPtrAtomic`) using `std::atomic<std::shared_ptr<std::unordered_map<int,int>>>`

---

## Build & Run

```sh
mkdir build && cd build
cmake ..            # or your preferred build system
make
./fibonacci_bench  # run benchmarks
```

No special dependencies beyond C++17 (or later).

---

## Implementations

### 1. Simple Recursion (`fibonacciRec`)
- Direct recursive definition without caching.
- Complexity: O(2^n).
- Used as baseline for pure overhead of recursion + async.

### 2. Memoized Recursion (`fibonacciMem`)
- Uses `std::unordered_map<int,int>` to store previously computed values.
- Recursion with cache lookup and insertion under a single map (not thread-safe).
- Complexity: O(n).

### 3. Asynchronous Recursion (`std::async`)
- Wraps calls to `fibonacciRec` and `fibonacciMem` in `std::async(std::launch::async, ...)`.
- Demonstrates cost of thread/task creation and synchronization via `std::future`.

### 4. Atomic Array-based Memoization (`fibonacciAtomic`)
- Fixed-size array `std::atomic<uint64_t> cache[MAX_N]` with sentinel `UINT64_MAX`.
- Lock-free memoization: atomic loads, recursive compute, CAS store.
- Thread-safe without external locks.

### 5. Non-atomic Array-based (`fibonacciNotAtomic`)
- Same as atomic version but with plain `uint64_t cache[MAX_N]`.
- Fast but not thread-safe.

### 6. Thread-safe Memoization (`fibonacciThred`)
- Uses `std::shared_mutex`: shared locks for reads, exclusive locks for first writes.
- Protects a single `std::unordered_map<int,int>` across threads.

### 7. Copy-on-Write Memoization (`fibonacciSharedPtrAtomic`)
- Maintains `std::atomic<std::shared_ptr<std::unordered_map<int,int>>> cachePtr`.
- Readers load snapshot; writers copy map, insert, CAS update pointer.
- Lock-free reads + dynamic map growth.

---

## Benchmark Results (example)
```
Time for async fibonacciMem        928400 ns
Time for NOT async fibonacciMem    27200  ns
Time for async fibonacciRec        1997540300 ns
Time for NOT async fibonacciRec    3962125000 ns
Time for atomic fibonacci          3800   ns
Time for NOT atomic fibonacci      1300   ns
Time for thread shared_mutex       3684800 ns
Time for thread atomic shared_ptr  2933600 ns
```

*Note:* Times will vary by hardware. Results illustrate relative overhead:
- Plain memoization is ~10× faster than async wrapper.
- Simple recursion dominates cost (multiple seconds for n≈40).
- Atomic array variant is extremely fast (few microseconds).
- Shared mutex version slower than atomic array due to lock overhead.
- Copy-on-write shared_ptr falls in between.

---

## Conclusions

1. **Recursion vs. Memoization**: Memoization reduces exponential recursion to linear complexity.
2. **Async Overhead**: `std::async` adds significant overhead; use only when parallelism benefit exceeds task creation cost.
3. **Lock-free vs. Locks**: Atomic array-based cache offers lowest latency and best scalability under high contention.
4. **Flexibility**: Copy-on-write with `atomic<shared_ptr>` trades off some speed for dynamic growth without explicit locks.
5. **Trade-offs**: Choose the method based on expected `n` range`, thread contention, and need for dynamic sizing.

---

