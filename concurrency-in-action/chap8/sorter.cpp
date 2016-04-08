#include <thread>
#include <iostream>
#include <list>
#include <future>
#include <vector>
#include <atomic>
#include <algorithm>
#include "../chap3/stack.hpp"

template <typename T>
struct sorter {  // 1
    struct chunk_to_sort {
        std::list<T> data;
        std::promise<std::list<T>> promises;
    };

    thread_stack<chunk_to_sort> chunks;  // 2
    std::vector<std::thread> threads;    // 3
    const unsigned max_thread_count;
    std::atomic<bool> end_of_data;

    sorter()
        : max_thread_count(std::thread::hardware_concurrency() - 1),
          end_of_data(false) {}

    ~sorter() {              // 4
        end_of_data = true;  // 5
        for (unsigned i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }
    }

    void try_sort_chunk() {
        std::shared_ptr<chunk_to_sort> chunk = chunks.pop();
        if (chunk) {
            sort_chunk(chunk);
        }
    }

    std::list<T> do_sort(std::list<T>& chunk_data) {  // 9
        if (chunk_data.empty()) {
            return chunk_data;
        }

        std::list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());
        const T& partition_val = *result.begin();

        typename std::list<T>::iterator divide_point =  // 10
            std::partition(chunk_data.begin(), chunk_data.end(),
                           [&](const T& val) { return val < partition_val; });

        chunk_to_sort new_lower_chunk;
        new_lower_chunk.data.splice(new_lower_chunk.data.end(), chunk_data,
                                    chunk_data.begin(), divide_point);

        std::future<std::list<T>> new_lower =
            new_lower_chunk.promises.get_future();
        chunks.push(std::move(new_lower_chunk));  // 11
        if (threads.size() < max_thread_count) {  // 12
            threads.push_back(std::thread(&sorter<T>::sort_thread, this));
        }

        std::list<T> new_higher(do_sort(chunk_data));

        result.splice(result.end(), new_higher);
        while (new_lower.wait_for(std::chrono::seconds(0)) !=
               std::future_status::ready) {  // 13
            try_sort_chunk();                // 14
        }
        result.splice(result.begin(), new_lower.get());
        return result;
    }

    void sort_chunk(const std::shared_ptr<chunk_to_sort>& chunk) {
        chunk->promises.set_value(do_sort(chunk->data));  // 15
    }

    void sort_thread() {
        while (!end_of_data) {          // 16
            try_sort_chunk();           // 17
            std::this_thread::yield();  // 18
        }
    }
};

template <typename T>
std::list<T> parallel_quick_sort(std::list<T> input) { // 19
    if (input.empty())
        return input;

    sorter<T> s;
    return s.do_sort(input);
}

int main() {
    std::list<int> lst = {4, 1, 3, 2, 6, 8, 7};
    lst = parallel_quick_sort(lst);
    for (auto&& c : lst) {
        std::cout << c << " ";
    }
    std::cout << std::endl;
    return 0;
}
