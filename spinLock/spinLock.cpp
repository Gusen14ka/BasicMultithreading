#include <iostream>
#include <atomic>
#include <mutex>
#include <ostream>
#include <thread>
#include <sstream>
#include <vector>
#include <chrono>


int counter = 0;

// SpinLockAtom and SpinLockAtomFlag are two implementations of a spinlock using atomic operations.
// SpinLockAtom uses std::atomic<bool> for locking, while SpinLockAtomFlag uses std::atomic_flag.
class SpinLockAtom {
    std::atomic<bool> atom{false};
    public:
    void lock(){
        while(atom.exchange(true)){
            std::this_thread::yield();
        }
    }
    void unlock(){
        atom.store(false, std::memory_order_release);
    }
};

class SpinLockAtomFlag{
    std::atomic_flag atom = ATOMIC_FLAG_INIT;
    public:
    void lock(){
        while(atom.test_and_set(std::memory_order_acquire)){
            std::this_thread::yield();
        }
    }
    void unlock(){
        atom.clear(std::memory_order_release);
    }
};

template<typename Sp>
void increaseSpinLockAtom(Sp& sl){
    for(int i = 0; i < 10000; i++){
        sl.lock();
        counter++;
        std::ostringstream ss;
        ss << std::this_thread::get_id();
        sl.unlock();
    }  
}

int main(){
    // Test the SpinLockAtom (atomic spinlock) implementation
    {
        SpinLockAtom sl;
        std::vector<std::thread> threads;
        for(int i = 0; i < 10; i++){
            threads.emplace_back(increaseSpinLockAtom<SpinLockAtom>, std::ref(sl));
        }
        auto start = std::chrono::high_resolution_clock::now();
        for(auto& t : threads){
            t.join();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Final counter value (SpinLockAtom): " << counter <<
                        "\nTime taken: " << end - start << std::endl;
        
    }
    counter = 0;
    // Test the SpinLockAtomFlag (atomic flag spinlock) implementation
    {
        SpinLockAtomFlag slf;
        std::vector<std::thread> threads;
        for(int i = 0; i < 10; i++){
            threads.emplace_back(increaseSpinLockAtom<SpinLockAtomFlag>, std::ref(slf));
        }
        auto start = std::chrono::high_resolution_clock::now();
        for(auto& t : threads){
            t.join();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Final counter value (SpinLockAtomFlag): " << counter <<
                        "\nTime taken: " << end - start << std::endl;
    }
    counter = 0;
    // Test mutex 
    {
        std::mutex mtx;
        std::vector<std::thread> threads;
        for(int i = 0; i < 10; i++){
            threads.emplace_back(increaseSpinLockAtom<std::mutex>, std::ref(mtx));
        }
        auto start = std::chrono::high_resolution_clock::now();
        for(auto& t : threads){
            t.join();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Final counter value (std::mutex): " << counter <<
                        "\nTime taken: " << end - start << std::endl;
    }

}
/*
Example output:
Final counter value (SpinLockAtom): 100000
Time taken: 251412300ns
Final counter value (SpinLockAtomFlag): 100000
Time taken: 250926400ns
Final counter value (std::mutex): 100000
Time taken: 593666200ns
This output shows that both SpinLockAtom and SpinLockAtomFlag perform similarly, while std::mutex is slower in this case.
*/