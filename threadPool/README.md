# ThreadPool
## Overview

This repository contains two simple C++ thread pool implementations:

1. **ThreadPool_jthread**  
   - Uses C++20 `std::jthread` with `std::stop_source`/`std::stop_token` for graceful shutdown.
   - Worker threads wait on a single `std::condition_variable` and exit when the stop token is requested.

2. **ThreadPool_thread**  
   - Uses classic `std::thread` with an `std::atomic<bool>` flag for stopping.
   - Worker threads wait on a single `std::condition_variable` and exit when the atomic flag is set.

Both pools accept arbitrary tasks via a templated `enqueue()` method returning a `std::future<R>`.

---

## Experiment

In `main.cpp`, we benchmark both thread pools with the same workload:

- **Number of worker threads**: 4
- **Number of tasks**: 8
- **Task work simulation**: each task sleeps for 100 ms × (i % 3 + 1)
- **Output**:  
  - Each task prints its index and thread id under a `cout` mutex.
  - The main thread collects futures and prints results.
  - Measure wall-clock time from pool creation to shutdown.

### Observed Results

```
Time for jthread pool = ~524 ms
Time for thread pool  = ~525 ms
```

Both implementations achieved virtually identical performance in this scenario.

---

## Conclusions

- **Correctness**: both pools handled tasks, returned correct results, and shut down cleanly.
- **Performance**: no measurable difference in this simple benchmark (<1 ms).
- **Design choices**:  
  - `std::jthread` + `stop_token` provides a modern, built-in cancellation mechanism.  
  - `std::thread` + `atomic<bool>` is equally valid and requires no special language feature beyond C++11.

Feel free to extend the benchmark with more tasks, longer workloads, and different shutdown strategies.
