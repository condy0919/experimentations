#include <thread>
#include <future>
#include <algorithm>
#include <numeric>
#include <atomic>

namespace v1 {
template <typename Iterator>
void parallel_partial_sum(Iterator first, Iterator last) {
    typedef typename Iterator::value_type value_type;

    struct process_chunk {
        void operator()(Iterator begin, Iterator last,
                        std::future<value_type>* previous_end_value,
                        std::promise<value_type>* end_value) {
            try {
                Iterator end = last;
                ++end;
                std::partial_sum(begin, end, begin);
                if (previous_end_value) {
                    value_type& addend = previous_end_value->get();
                    *last += addend;
                    if (end_value) {
                        end_value->set_value(*last);
                    }
                    std::for_each(begin, last, [addend](value_type& item) {
                        item += addend;
                    });
                } else if (end_value) {
                    end_value->set_value(*last);
                }
            } catch (...) {
                if (end_value) {
                    end_value->set_exception(std::current_exception());
                } else {
                    throw;
                }
            }
        }
    };
    const unsigned long length = std::distance(first, last);
    if (!length)
        return last;

    const unsigned long min_per_thread = 25;
    const unsigned long max_threads =
        (length + min_per_thread - 1) / min_per_thread;
    const unsigned long hardware_threads = std::thread::hardware_concurrency();
    const unsigned long num_threads =
        std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

    const unsigned long block_size = length / num_threads;

    typedef typename Iterator::value_type value_type;
    std::vector<std::thread> threads(num_threads - 1);
    std::vector<std::promise<value_type>> end_value(num_threads - 1);
    previous_end_value.reserve(num_threads - 1);
    join_threads joiner(threads);

    Iterator block_start = first;
    for (unsigned long i = 0; i < num_threads - 1; ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] =
            std::thread(process_chunk(), block_start, block_end,
                        (i != 0) ? previous_end_value[i - 1] : 0, &end_value);
        block_start = block_end;
        ++block_start;
        previous_end_value.push_back(end_value[i].get_future())
    }
    Iterator final_element = block_start;
    std::advance(final_element, std::distance(block_start, last) - 1);
    process_chunk()(block_start, final_element,
                    (num_threads > 1) ? &previous_end_value.back() : 0, 0);
}
}
