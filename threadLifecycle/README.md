# Multithreaded Sleep Demo

## Purpose

This small program demonstrates the basics of multithreading in C++:
- How to spawn multiple worker threads.
- How to safely output from threads without interleaving.
- How to coordinate threads and wait for their completion.

Such exercises help you understand thread lifecycles, synchronization primitives, and the behavior of concurrent execution in practice.

## How It Works

1. **User Input**  
   - Reads a positive integer **N** from the console: the number of threads to launch.

2. **Thread Function**  
   - Each thread logs its start message under a mutex to avoid mixing outputs.
   - It then sleeps for a random duration (100–500 ms), simulating work.
   - After waking up, it logs its finish message under the same mutex.

3. **Synchronization**  
   - A single `std::mutex` guards all interactions with `std::cout`.
   - Threads run truly concurrently; the sleep happens outside the locked sections.

4. **Thread Management**  
   - Threads are stored in a `std::vector<std::thread>`.
   - The main thread calls `join()` on each to ensure all complete before printing the final message.
