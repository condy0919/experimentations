#include <thread>
#include <atomic>
#include <boost/thread/barrier.hpp>
#include <mutex>
#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>

std::mutex io_mutex;

void foo(boost::barrier& cur_barrier, std::atomic<int>& current) {
    ++current;
    cur_barrier.wait();
    std::lock_guard<std::mutex> guard(io_mutex);
    std::cout << current << std::endl;
}

int main() {
    boost::barrier bar(2);
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
