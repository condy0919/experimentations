#include <iostream>
#include <functional>
#include <memory>
#include <thread>
#include <future>

void print_int(std::future<int>& fut) {
    int x = fut.get();
    std::cout << "value = " << x << std::endl;
}

int main() {
    std::promise<int> foo;
    std::promise<int> bar = std::promise<int>(std::allocator_arg, std::allocator<int>());
    std::future<int> fut = bar.get_future();
    std::thread th(print_int, std::ref(fut));
    bar.set_value(20);
    th.join();
    return 0;
}
