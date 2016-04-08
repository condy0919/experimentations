#include <thread>
#include <future>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <exception>
#include <numeric>
#include <vector>

template <typename Iterator, typename T>
struct accumulate_block {
    T operator()(Iterator first, Iterator last) {  // 1
        return std::accumulate(first, last, T());  // 2
    }
};

template <typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    const unsigned long length = std::distance(first, last);

    if (length == 0)
        return init;

    const unsigned long min_per_thread = 25;
    const unsigned long max_threads =
        (length + min_per_thread - 1) / min_per_thread;
    const unsigned long hardware_threads = std::thread::hardware_concurrency();
    const unsigned long num_threads =
        std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

    const unsigned long block_size = length / num_threads;

    std::vector<std::future<T>> futures(num_threads - 1);  // 3
    std::vector<std::thread> threads(num_threads - 1);

    Iterator block_start = first;
    for (unsigned long i = 0; i < num_threads - 1; ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        std::packaged_task<T(Iterator, Iterator)> task(  // 4
            accumulate_block<Iterator, T>());
        futures[i] = task.get_future();                                     // 5
        threads[i] = std::thread(std::move(task), block_start, block_end);  // 6
        block_start = block_end;
    }
    T last_result = accumulate_block<Iterator, T>()(block_start, last);  // 7

    std::for_each(threads.begin(), threads.end(),
                  std::mem_fn(&std::thread::join));
    T result = init;  // 8
    for (unsigned long i = 0; i < num_threads - 1; ++i) {
        result += futures[i].get();  // 9
    }
    result += last_result;  // 10
    return result;
}
