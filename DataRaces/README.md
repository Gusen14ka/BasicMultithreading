# DATARACES: Atomic vs Mutex vs Race Benchmark

This project demonstrates the performance and correctness differences between three approaches to incrementing a shared counter across multiple threads in C++:

1. **Race Condition**  
   - No synchronization.  
   - Fastest, but produces incorrect results due to data races (undefined behavior).

2. **`std::mutex`**  
   - Each increment is protected by a `std::mutex` lock.  
   - Produces correct results, but incurs high overhead from kernel-level locks.

3. **`std::atomic`**  
   - Uses atomic operations (`fetch_add` / `operator++`) with different memory orders.  
   - Balances correctness with performance.  
   - `memory_order_seq_cst` (default) vs `memory_order_relaxed` benchmarks.

## Files

- **`DataRaces.cpp`**  
  C++ source with four test functions:
  - `f_race`: no sync.
  - `f_mut_race`: uses `std::mutex`.
  - `f_atomic_race`: uses `std::atomic` default.
  - `f_atomic_relaxed_race`: uses `std::atomic` with `memory_order_relaxed`.

- **`README.md`**  
  This file.

## How to build and run

```bash
mkdir build && cd build
cmake ..
make
./benchmark
```


