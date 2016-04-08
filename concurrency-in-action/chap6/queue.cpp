#include <thread>
#include <mutex>
#include <memory>
#include <queue>
#include <condition_variable>

namespace version1 {

template <typename T>
class threadsafe_queue {
private:
    mutable std::mutex m;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_queue() {}

    void push(T val) {
        std::lock_guard<std::mutex> guard(m);
        data_queue.push(std::move(val));
        data_cond.notify_one();
    }

    void wait_and_pop(T& val) {
        std::unique_lock<std::mutex> lk(m);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        val = std::move(data_queue.front());
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(m);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        auto res = std::make_shared<T>(std::move(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool try_pop(T& val) {
        std::lock_guard<std::mutex> lk(m);
        if (data_queue.empty())
            return false;
        val = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lk(m);
        if (data_queue.empty())
            return false;
        auto res = std::make_shared<T>(std::move(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool empty() const {
        std::lock_guard<std::mutex> guard(m);
        return data_queue.empty();
    }
};
}
