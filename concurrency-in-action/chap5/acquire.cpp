#include <atomic>
#include <array>
#include <thread>
#include <functional>
#include <algorithm>
#include <assert.h>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x() {
    x.store(true, std::memory_order_release);
}

void write_y() {
    y.store(true, std::memory_order_release);
}

void read_x_then_y() {
    while (!x.load(std::memory_order_acquire));
    if (y.load(std::memory_order_acquire))
        ++z;
}

void read_y_then_x() {
    while (!y.load(std::memory_order_acquire));
    if (x.load(std::memory_order_acquire))
        ++z;
}

int main() {
    x = false;
    y = false;
    z = 0;
    std::array<std::function<void(void)>, 4> funcs = {
        write_x,
        write_y,
        read_x_then_y,
        read_y_then_x
    };
    std::thread tds[4];
    for (int i = 0; i < funcs.size(); ++i) {
        tds[i] = std::thread(funcs[i]);
    }
    std::for_each(std::begin(tds), std::end(tds), std::mem_fn(&std::thread::join));
    assert(z.load() != 0);
    return 0;
}
