#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>

std::mutex mut;
std::queue<int> data_queue;
std::condition_variable data_cond;

constexpr bool more_data_to_prepare() {
    return true;
}

void data_preparation_thread() {
    while (more_data_to_prepare()) {
        constexpr const int data = 1;
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }
}

void data_processing_thread() {
    while (true) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [](){ return !data_queue.empty(); });
        const int data_chunk = data_queue.front();
        data_queue.pop();
        lk.unlock();
        //process(data);
        //if (is_last_chunk(data))
        //    break;
    }
}
