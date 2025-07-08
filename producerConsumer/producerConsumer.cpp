#include <condition_variable>
#include <cstddef>
#include <deque>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stop_token>
#include <string>
#include <thread>

constexpr std::size_t N = 5;
constexpr std::chrono::milliseconds max_wait{100};
std::deque<std::string> deq;
std::mutex mut;
std::condition_variable notFull, notEmpty;

void producer(){
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    std::string output = ss.str();
    for(int i = 0; i < 10; i++){
        std::unique_lock<std::mutex> lk(mut);
        notFull.wait(lk, []{return deq.size() < N;});
        deq.push_back(output + " " + std::to_string(i));
        std::cout << "Write " << output + " " + std::to_string(i) << std::endl;
        notEmpty.notify_one();
    }
}

void consumer(std::stop_token sToken){
    while(true){
        if (sToken.stop_requested() && deq.empty()) {
            break;
        }
        std::unique_lock<std::mutex> lk(mut);
        notEmpty.wait_for(lk, max_wait, [&sToken]{return !deq.empty() || sToken.stop_requested();});
        while(!deq.empty()){
            auto item = std::move(deq.front());
            deq.pop_front();
            std::cout << "Read " << item << std::endl;
        }
        notFull.notify_one();
    }
}

int main(){
    std::thread prod(producer);
    std::jthread con(consumer);
    prod.join();
    con.request_stop();
}