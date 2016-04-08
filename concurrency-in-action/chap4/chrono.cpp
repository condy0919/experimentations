#include <chrono>
#include <iostream>
#include <future>

int some_task() {
    std::chrono::seconds s(1);
    std::this_thread::sleep_for(s);
    return 0;
}

int main() {
    std::chrono::milliseconds ms = std::chrono::milliseconds(10);
    std::chrono::seconds s =
        std::chrono::duration_cast<std::chrono::seconds>(ms);

    // std::cout << ms << " " << s << std::endl;

    std::future<int> f = std::async(std::launch::async, some_task);
    std::chrono::seconds tm(2);
    switch (auto state = f.wait_for(tm)) {
        case std::future_status::ready:
            std::cout << "ready\n";
            std::cout << f.get() << std::endl;
            break;

        case std::future_status::deferred:
            std::cout << "deferred\n";
            std::cout << f.get() << std::endl;
            break;

        case std::future_status::timeout:
            std::cout << "timeout\n";
            std::cout << f.get() << std::endl;
            break;
    }
    return 0;
}
