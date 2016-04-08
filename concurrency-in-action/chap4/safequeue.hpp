#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template <typename T>
class threadsafe_queue {
private:
    mutable std::mutex mtx;
    std::queue<T> data_queue;
    std::condition_variable cond;

public:
    threadsafe_queue() {}
    threadsafe_queue(const threadsafe_queue& rhs) {
        std::lock_guard<std::mutex> guard(rhs.mtx);
        data_queue = rhs.data_queue;
    }

    void push(T val) {
        std::lock_guard<std::mutex> guard(mtx);
        data_queue.push(std::move(val));
        cond.notify_one();
    }

    void wait_and_pop(T& val) {
        std::unique_lock<std::mutex> lk(mtx);
        cond.wait(lk, [this]{ return !data_queue.empty(); });
        val = data_queue.front();
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mtx);
        cond.wait(lk, [this]{ return !data_queue.empty(); });
        auto res = std::make_shared<T>(data_queue.front());
        data_queue.pop();
        return res;
    }

    bool try_pop(T& val) {
        std::lock_guard<std::mutex> lk(mtx);
        if (data_queue.empty()) {
            return false;
        }
        val = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> guard(mtx);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        auto res = std::make_shared<T>(data_queue.front());
        data_queue.pop();
        return res;
    }

    bool empty() const {
        std::lock_guard<std::mutex> guard(mtx);
        return data_queue.empty();
    }
};
