#include <atomic>
#include <iostream>

int main() {
    std::atomic_flag f = ATOMIC_FLAG_INIT;
    f.clear(std::memory_order_release);
    bool x = f.test_and_set();
}
