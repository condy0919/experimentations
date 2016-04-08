#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <iostream>
#include <cstdlib>
#include <ctime>

namespace inthreadSafe {
template <typename T>
class queue {
public:
    std::mutex mtx;
    std::queue<T> data_queue;
    std::condition_variable cond;

public:
    queue() {}

    queue(const queue& rhs) {}

    queue& operator=(const queue& rhs) = delete;

    void push(T val) {
        std::lock_guard<std::mutex> guard(mtx);
        data_queue.push(std::move(val));
        cond.notify_one();
    }

    bool try_pop(T& val) {}

    std::shared_ptr<T> try_pop() {}

    void wait_and_pop(T& val) {
        std::unique_lock<std::mutex> lk(mtx);
        cond.wait(lk, [this]{ return !data_queue.empty(); });
        std::cout << "position1\n";
        val = data_queue.front();
        data_queue.pop();
    }
    //std::shared_ptr<T> wait_and_pop() {}

    bool empty() const {}
};
}

int main() {
    using namespace inthreadSafe;
    queue<int> Q;
    int val = -1;
    auto thread_a = std::thread(&queue<int>::wait_and_pop, &Q, std::ref(val));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "send notification\n";
    Q.cond.notify_one();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::cout << "push data and send notification\n";
    Q.push(1);
    Q.cond.notify_one();

    while (true) {
        std::this_thread::yield();
    }
    return 0;
}
