#include <thread>
#include <atomic>
#include <future>
#include <algorithm>
#include <numeric>
#include <vector>
#include <utility>
#include <memory>

class join_threads {
private:
    std::vector<std::thread>& threads;

public:
    join_threads(std::vector<std::thread>& tds) : threads(tds) {}
    ~join_threads() {
        for (auto&& t : threads) {
            if (t.joinable())
                t.join();
        }
    }
};

template <typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f) {
    const unsigned long length = std::distance(first, last);

    if (!length)
        return;

    const unsigned long min_per_thread = 25;
    const unsigned long max_threads =
        (length + min_per_thread - 1) / min_per_thread;
    const unsigned long hardware_threads = std::thread::hardware_concurrency();
    const unsigned long num_threads =
        std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

    const unsigned long block_size = length / num_threads;
    std::vector<std::future<void>> futures(num_threads - 1);
    std::vector<std::thread> threads(num_threads - 1);
    join_threads joiner(threads);

    Iterator block_start = first;
    for (unsigned long i = 0; i < num_threads - 1; ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        std::packaged_task<void(void)> task(
            [=] { std::for_each(block_start, block_end, f); });
        futures[i] = task.get_future();
        threads[i] = std::thread(std::move(task));
        block_start = block_end;
    }
    std::for_each(block_start, last, f);
    for (unsigned long i = 0; i < num_threads - 1; ++i) {
        futures[i].get();
    }
}

namespace v2 {
template <typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f) {
    const unsigned long length = std::distance(first, last);

    if (!length)
        return;

    const unsigned long min_per_thread = 25;
    if (length < 2 * min_per_thread) {
        std::for_each(first, last, f);
    } else {
        const Iterator mid_point = first + length / 2;
        std::future<void> first_half = std::async(&parallel_for_each<Iterator, Func>, first, mid_point, f);
        parallel_for_each(mid_point, last, f);
        first_half.get();
    }
}
}
