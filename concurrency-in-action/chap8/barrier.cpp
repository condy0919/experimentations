#include <iostream>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <utility>
#include <memory>
#include <type_traits>
#include <thread>
#include <functional>
#include <vector>

#include <unistd.h>
#include <pthread.h>

class base {
public:
    virtual ~base() {}
};

template <typename Func>
class at_exit : public base {
private:
    Func fn;

public:
    at_exit(Func f) : fn(std::move(f)) {}
    virtual ~at_exit() {
        fn();
    }
};

class guard {
public:
    template <typename T, typename U, typename = std::enable_if_t<std::is_same<
                                          std::decay_t<U>, int (*)(T*)>::value>>
    guard(T& obj, U ctor, U dtor) {
        ctor(std::addressof(obj));
        auto lambda = [=, ptr = std::addressof(obj)]() {
            dtor(ptr);
        };
        base_ptr =
            std::make_unique<at_exit<decltype(lambda)>>(std::move(lambda));
    }
    ~guard() {
    }

private:
    std::unique_ptr<base> base_ptr;
};

class barrier {
private:
    pthread_barrier_t barrier_;

public:
    barrier(std::size_t cnt) {
        pthread_barrierattr_t attr;
        guard gd(attr, pthread_barrierattr_init, pthread_barrierattr_destroy);
        pthread_barrier_init(&barrier_, &attr, cnt);
    }

    ~barrier() {
        pthread_barrier_destroy(&barrier_);
    }

    void wait() {
        pthread_barrier_wait(&barrier_);
    }
};

std::mutex io_mutex;

void foo(barrier& cur_barrier, std::atomic<int>& current) {
    ++current;
    cur_barrier.wait();
    std::lock_guard<std::mutex> lk(io_mutex);
    std::cout << current << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main() {
    barrier bar(2);
    std::atomic<int> current(0);
    std::vector<std::thread> threads;

    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(foo, std::ref(bar), std::ref(current));
    }
    for (auto&& t : threads) {
        if (t.joinable())
            t.join();
    }
    return 0;
}
