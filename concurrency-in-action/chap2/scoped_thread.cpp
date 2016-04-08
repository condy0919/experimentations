#include <iostream>
#include <thread>
#include <utility>
#include <exception>
#include <stdexcept>
#include <functional>
#include <vector>
#include <algorithm>

class scoped_thread {
private:
    std::thread t;

public:
    explicit scoped_thread(std::thread t_) : t(std::move(t_)) {
        if (!t.joinable())
            throw std::logic_error("No thread");
    }

    ~scoped_thread() {
        t.join();
    }

    scoped_thread(const scoped_thread&) = delete;
    scoped_thread& operator=(const scoped_thread&) = delete;
};

void func() {
    std::cout << __func__ << '\n';
}

void f() {
    scoped_thread t{std::thread(func)};
}

void do_work(int id) {
    std::cout << id << '\n';
}

int main() {
    //f();
    std::vector<std::thread> threads;
    for (int i = 0; i < 20; ++i) {
        threads.emplace_back(do_work, i);
    }
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    return 0;
}
