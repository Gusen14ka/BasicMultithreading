# Producer-Consumer with Batching and Timeout

## Purpose

This example demonstrates a hybrid producer–consumer pattern in C++ where:
- Producers generate items and push them into a bounded buffer.
- Consumers process buffer contents either when a batch size threshold is reached or when a timeout expires.
- A stop token cleanly stops the consumer once production finishes.

It highlights key concepts:
- `std::mutex` and `std::condition_variable` for synchronization.
- `wait_for` with a predicate combining buffer size and stop requests.
- Graceful shutdown of `std::jthread` using `std::stop_token`.

## Parameters

- **Buffer Capacity (N)**: Maximum items the buffer can hold.
- **Timeout (`max_wait`)**: Maximum wait duration before processing whatever is in the buffer.
- **Batch Processing**: Consumer drains the entire buffer on wake-up.

## How It Works

1. **Producer**  
   - Generates a fixed number of items, waiting if the buffer is full.
   - Signals the consumer via `notEmpty` after each push.

2. **Consumer**  
   - Uses `wait_for(max_wait)` on `notEmpty` with a predicate:
     - Buffer non-empty, or stop requested.
   - Upon wake-up, drains the buffer and signals `notFull`.
   - Exits when stop token is requested and buffer is empty.

3. **Shutdown**  
   - Main thread joins the producer.
   - Requests stop on the consumer thread.
   - Optionally notifies `notEmpty` to wake the consumer immediately.

## Building

```bash
g++ -std=c++20 -pthread -o pc_demo main.cpp
./pc_demo
```

## Notes

- The batch+timeout approach balances throughput and latency.
- Including the stop token in the wait predicate prevents hanging on shutdown.