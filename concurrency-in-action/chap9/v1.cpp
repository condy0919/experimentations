#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include <vector>
#include <condition_variable>

#include <queue>
#include <boost/lockfree/queue.hpp>

class thread_pool {
private:
    std::atomic_bool done;
    //boost::lockfree::queue<std::function<void()>> work_queue;
    std::queue<std::function<void()>> work_queue;
    std::vector<std::thread> threads;
    //join_threads joiner;

    void work_thread() {
        while (!done) {
            std::function<void()> task;
            //if (work_queue.pop(task)) {
            if (!work_queue.empty()) {
                task = work_queue.front();
                work_queue.pop();
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }

public:
    thread_pool()
        : done(false)
    {
        const unsigned thread_count = std::thread::hardware_concurrency();

        try {
            for (unsigned i = 0; i < thread_count; ++i) {
                threads.emplace_back(&thread_pool::work_thread, this);
            }
        } catch (...) {
            done = true;
            throw;
        }
    }

    ~thread_pool() {
        done = true;
    }

    template <typename Func>
    void submit(Func f) {
        work_queue.push(std::function<void()>(f));
    }
};
