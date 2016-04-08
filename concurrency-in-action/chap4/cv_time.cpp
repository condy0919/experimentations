#include <condition_variable>
#include <mutex>
#include <chrono>

std::condition_variable cv;
bool done;
std::mutex mtx;

bool wait_loop() {
    const auto timeout =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    std::unique_lock<std::mutex> lk(mtx);
    while (!done) {
        if (cv.wait_until(lk, timeout) == std::cv_status::timeout)
            break;
    }
    return done;
}

int main() {
    return 0;
}
