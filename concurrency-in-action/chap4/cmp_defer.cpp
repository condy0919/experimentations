#include <future>
#include <iostream>
#include <thread>
#include <mutex>

int foo(int x) {
    return x + 1;
}

int main() {
    std::future<int> f1 = std::async(std::launch::deferred, foo, 1);
    std::cout << f1.get() << '\n';
    return 0;
}
