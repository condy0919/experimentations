#include <atomic>
#include <mutex>
#include <thread>
#include <iostream>

class spinlock_mutex {
private:
    std::atomic_flag flag;

public:
    spinlock_mutex() : flag(ATOMIC_FLAG_INIT) {}

    void lock() {
        while (flag.test_and_set(std::memory_order_acquire))
            ;
    }
    
    void unlock() {
        flag.clear(std::memory_order_release);
    }
};

spinlock_mutex mtx;

void foo(int id) {
    std::lock_guard<spinlock_mutex> guard(mtx);
    std::cout << id << '\n';
}

int main() {
    std::thread t1(foo, 1), t2(foo, 2);
    t1.join();
    t2.join();
    return 0;
}
