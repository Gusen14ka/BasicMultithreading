# Spinlock vs Mutex Performance Comparison

## Experiment Description

This repository contains a simple benchmark to compare the performance and correctness of two custom spinlock implementations against `std::mutex` in C++. The goal is to measure execution times for a high-contention scenario on a shared counter.

### Implementations Tested

1. **SpinLockAtom**  
   - Uses `std::atomic<bool>` for locking with `exchange` and `yield()` in a spin loop.

2. **SpinLockAtomFlag**  
   - Uses `std::atomic_flag` for locking with `test_and_set` and `yield()` in a spin loop.

3. **std::mutex**  
   - Standard C++ mutex for reference.

## Benchmark Setup

- **Shared counter**: `int counter = 0`.
- **Thread count**: 10 threads.
- **Increments per thread**: 10,000.
- **Total expected increments**: 100,000.

For each lock type:
1. Spawn 10 threads, each executing:
   - Acquire lock
   - Increment `counter`
   - Release lock
   - Repeat 10,000 times
2. Join all threads
3. Measure elapsed time using `std::chrono::high_resolution_clock`
4. Print final counter value and time taken (nanoseconds)

## Results

\`\`\`
Final counter value (SpinLockAtom): 100000
Time taken: 251412300ns

Final counter value (SpinLockAtomFlag): 100000
Time taken: 250926400ns

Final counter value (std::mutex): 100000
Time taken: 593666200ns
\`\`\`

- Both spinlock variants produce the correct final counter (100,000).
- Execution times:
  - **SpinLockAtom**: ~251 ms
  - **SpinLockAtomFlag**: ~251 ms
  - **std::mutex**: ~594 ms

## Conclusions

- **Correctness**: All three mechanisms ensure data integrity under concurrent access.
- **Performance**: Spinlocks (`std::atomic<bool>` and `std::atomic_flag`) are ~2.3× faster than `std::mutex` for very short critical sections under moderate thread contention.
- **SpinLockAtom vs SpinLockAtomFlag**: Nearly identical performance; `atomic_flag` is a more lightweight, canonical choice for spinlocks.
- **Usage Recommendations**:
  - Use **spinlocks** for extremely short, high-frequency critical sections where context switch overhead of `std::mutex` is too costly.
  - Use **`std::mutex`** for longer, less predictable critical sections to avoid busy-waiting and CPU waste.
