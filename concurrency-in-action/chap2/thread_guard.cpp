#include <thread>
#include <iostream>

using namespace std;

class thread_guard {
private:
    std::thread& t;

public:
    explicit thread_guard(std::thread& t_) : t(t_) {}

    ~thread_guard() {
        if (t.joinable()) {
            t.join();
        }
    }

    thread_guard(const thread_guard&) = delete;
    thread_guard& operator=(const thread_guard&) = delete;
};

void my_func() {
    cout << __func__ << endl;
}

int main() {
    std::thread t(my_func);
    thread_guard g(t);
    std::cout << t.get_id() << '\n';
    return 0;
}
