#include "../chap4/safequeue.hpp"

#include <thread>
#include <iostream>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <future>
#include <vector>
#include <queue>
#include <memory>
#include <numeric>

class function_wrapper {
private:
    struct impl_base {
        virtual void call() = 0;
        virtual ~impl_base() {}
    };

    std::unique_ptr<impl_base> impl;
    template <typename F>
    struct impl_type : impl_base {
        F f;
        impl_type(F&& f_) : f(std::move(f_)) {}
        void call() {
            f();
        }
    };

public:
    template <typename F>
    function_wrapper(F&& f)
        : impl(std::make_unique<impl_type<F>>(std::move(f))) {}

    void operator()() {
        impl->call();
    }

    function_wrapper() = default;

    function_wrapper(function_wrapper&& rhs) : impl(std::move(rhs.impl)) {}

    function_wrapper& operator=(function_wrapper&& rhs) {
        impl = std::move(rhs.impl);
        return *this;
    }

    function_wrapper(const function_wrapper&) = delete;
    function_wrapper& operator=(const function_wrapper&) = delete;
};

class thread_joiner {
private:
    std::vector<std::thread>& threads;

public:
    thread_joiner(std::vector<std::thread>& tds)
        : threads(tds)
    {}
    ~thread_joiner() {
        for (auto&& t : threads) {
            if (t.joinable())
                t.join();
        }
    }
};

class thread_pool {
private:
    std::atomic_bool done;
    std::mutex mtx;
    threadsafe_queue<function_wrapper> pool_work_queue;
    typedef std::queue<function_wrapper> local_queue_type;
    static thread_local std::unique_ptr<local_queue_type> local_work_queue; // 2
    std::vector<std::thread> threads;
    thread_joiner joiner;

    void work_thread() {
        local_work_queue.reset(new local_queue_type);
        while (!done) {
            run_pending_task();
        }
    }

public:
    thread_pool() : done(false), joiner(threads) {
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
    std::future<std::result_of_t<Func()>> submit(Func f) {
        using result_type = std::result_of_t<Func()>;

        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res = task.get_future();
        if (local_work_queue) {
            local_work_queue->push(std::move(task));
        } else {
            pool_work_queue.push(std::move(task));
        }
        return res;
    }

    void run_pending_task() {
        function_wrapper task;
        if (local_work_queue && !local_work_queue->empty()) {
            task = std::move(local_work_queue->front());
            local_work_queue->pop();
            task();
            std::cout << "executed in local work queue\n";
        } else if (pool_work_queue.try_pop(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }
};

thread_local std::unique_ptr<thread_pool::local_queue_type> thread_pool::local_work_queue;

template <typename Iterator, typename T>
struct accumulate_block {
    T operator()(Iterator beg, Iterator end, T init) {
        return std::accumulate(beg, end, init);
    }
};

template <typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    const unsigned long length = std::distance(first, last);

    if (!length)
        return init;

    const unsigned long block_size = 2;
    const unsigned long num_block = (length + block_size - 1) / block_size;

    std::vector<std::future<T>> futures(num_block - 1);
    thread_pool pool;

    Iterator block_start = first;
    for (unsigned long i = 0; i < num_block - 1; ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        // futures[i] = pool.submit(accumulate_block<Iterator, T>());
        futures[i] = pool.submit([=] {
            return accumulate_block<Iterator, T>()(block_start, block_end, 0);
        });
        block_start = block_end;
    }
    T last_result = accumulate_block<Iterator, T>()(block_start, last, 0);
    T result = init;
    for (unsigned long i = 0; i < num_block - 1; ++i) {
        result += futures[i].get();
    }
    result += last_result;
    return result;
}

int main() {
    std::vector<int> a = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    int ans = parallel_accumulate(a.begin(), a.end(), 0);
    std::cout << ans << '\n';
    return 0;
}
