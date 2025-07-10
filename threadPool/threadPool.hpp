#pragma once
#include <atomic>
#include <deque>
#include <memory>
#include <stdexcept>
#include <vector>
#include <stop_token>
#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <type_traits>
#include <future>
#include <utility>

class ThreadPool_jthread{
    public:
    ThreadPool_jthread(std::size_t n) : N_(n){
        for(auto i = 0; i < N_; i++){
            threads.emplace_back(
                [this](std::stop_token st){this->threadFunc(st);}
            );
        }
    }
    ~ThreadPool_jthread(){
        shoutdown();
    }
    // Function to join all threads
    void shoutdown(){
        if(stopSource.stop_possible()){
            stopSource.request_stop();
            cv_.notify_all();
            for(auto& t : threads){
                t.join();
            }
            if(queue_.empty()){
                std::cout << "All right! Queue is empty and threads ara joined\n";
            }
        }
    }
    // Function to add task in threadPool
    template<typename F, typename ...Args>
    auto enqueue(F&& f, Args... args) -> std::future<std::invoke_result_t<F, Args...>>{
        // Check for shoutdown threadPool
        // If stopSource is stopped, throw exception
        if(stopSource.stop_requested()){
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        using R = std::invoke_result_t<F, Args...>;
        // Create a packaged task to store the result of the function
        // Make it shared to be able to use it in the thread
        // and return future to the user
        // Use std::forward and mutable to preserve the value category of the arguments
        // and allow perfect forwarding
        auto packPtr = std::make_shared<std::packaged_task<R()>>(
            [f = std::forward<F>(f), ...args = std::forward<Args>(args)] () mutable {
                return f(args...);
            }
        );
        auto res = packPtr->get_future();
        // Lock the mutex to safely access the queue
        // and then notify to wake up one of the threads
        {
            std::lock_guard<std::mutex> lk(mtx_);
            queue_.emplace_back([packPtr]{(*packPtr());});
        }
        cv_.notify_one();
        return res;
    }


    private:
    // Queue for input tasks
    std::deque<std::function<void()>> queue_;
    // Mutex for safety access to queue_
    std::mutex mtx_;
    // Condition variable for safety access to queue_
    std::condition_variable cv_;
    // Number of used threads
    std::size_t N_;
    // Vector of threads
    std::vector<std::jthread> threads;
    // Stop logic for threads
    std::stop_source stopSource;

    // Wrap function for threads
    void threadFunc(std::stop_token sToken){
        while(true){
            std::unique_lock lk(mtx_);
            cv_.wait(lk, [this, &sToken]{return !this->queue_.empty() || sToken.stop_requested();});
            if(queue_.empty() && sToken.stop_requested()){
                break;
            }
            auto task = std::move(queue_.front());
            queue_.pop_front();
            lk.unlock();
            task();
        }
    }

};

class ThreadPool_thread{
    private:
    // Queue for input tasks
    std::deque<std::function<void()>> queue_;
    // Mutex for safety access to queue_
    std::mutex mtx_;
    // Condition variable for safety access to queue_
    std::condition_variable cv_;
    // Number of used thread
    std::size_t N_;
    // Vector of used threads
    std::vector<std::thread> threads;
    // Stop flag
    std::atomic<bool> atom{false};

    // Wrap function for threads
    void threadFunc(){
        while(true){
            std::unique_lock lk(mtx_);
            cv_.wait(lk, [this]{return !queue_.empty() || atom.load(std::memory_order_relaxed);});
            if(queue_.empty() && atom.load(std::memory_order_acquire)){
                break;
            }
            auto task = std::move(queue_.front());
            queue_.pop_front();
            lk.unlock();
            task();
        }
    }
    public:
    ThreadPool_thread(std::size_t n) : N_(n){
        for(int i = 0; i < N_; i++){
            threads.emplace_back(
                [this]{this->threadFunc();}
            );
        }
    }
    ~ThreadPool_thread(){
        shoutdown();
    }
    void shoutdown(){
        if(!atom.exchange(true)){
            atom.store(true, std::memory_order_release);
            cv_.notify_all();
            for(auto& t: threads){
                t.join();
            }
            if(queue_.empty()){
                std::cout << "All right! Queue is empty and threads ara joined\n";
            }
        }
    }
    template<typename F, typename ...Args>
    auto enqueue(F&& f, Args&& ...args) -> std::future<std::invoke_result_t<F, Args...>>{
        using R = std::invoke_result_t<F, Args...>;

        // Check for shoutdown threadPool
        if(atom.load(std::memory_order_acquire)){
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        auto packPtr = std::make_shared<std::packaged_task<R()>>(
            [f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable
        {
            return f(args...);
        });
        auto res = packPtr->get_future();
        {
            std::lock_guard lk(mtx_);
            queue_.emplace_back([packPtr]{(*packPtr)();});
        }
        cv_.notify_one();
        return res;
    }
};