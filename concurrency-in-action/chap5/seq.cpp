#include <vector>
#include <atomic>
#include <thread>
#include <iostream>

std::vector<int> data;
std::atomic<bool> data_ready(false);

void reader_thread() {
    while (!data_ready.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "The answer = " << data[0] << '\n';
}

void writer_thread() {
    data.push_back(42);
    data_ready = true;
}

int main() {
    std::thread t1(writer_thread), t2(reader_thread);
    t1.join();
    t2.join();
    return 0;
}
