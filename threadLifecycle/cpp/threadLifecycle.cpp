#include <mutex>
#include <thread>
#include <iostream>
#include <chrono>
#include <random>
#include <vector>

std::mutex lock;

void threadFunc(std::mt19937& gen){
    auto id = std::this_thread::get_id();
    {
    std::lock_guard<std::mutex>lg (lock);
    std::cout << "Thread " << id << " started\n";
    }

    std::uniform_int_distribution<int> dist(100, 500);
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));

    {
    std::lock_guard<std::mutex>lg (lock);
    std::cout << "Thread " << id << " finished\n";
    }
}


int main(){
    std::mt19937 gen(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));
    auto N(0);
    std::cout << "Enter a number of threads: ";
    std::cin >> N;

    if(N <= 0) {
        std::cerr << "Number of threads must be positive." << std::endl;
        return 1;
    }

    std::vector<std::thread> threads;
    for(int i = 0; i < N; i++){
        threads.push_back(std::thread(threadFunc, std::ref(gen)));
    }
    for(auto& t : threads){
        t.join();
    }

    std::cout << "All threads are done!\n";

}