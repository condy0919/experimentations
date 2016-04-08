#include <mutex>
#include <thread>
#include <iostream>

struct some_big_data {};
void swap(some_big_data& lhs, some_big_data& rhs);

class X {
private:
    some_big_data some_detail;
    std::mutex m;

public:
    X(const some_big_data& rhs) : some_detail(rhs) {}

    friend void swap(X& lhs, X& rhs) {
        if (&lhs == &rhs)
            return;
        std::lock(lhs.m, rhs.m);
        std::lock_guard<std::mutex> guard_a(lhs.m, std::adopt_lock);
        std::lock_guard<std::mutex> guard_b(rhs.m, std::adopt_lock);
        swap(lhs.some_detail, rhs.some_detail);
    }
};
