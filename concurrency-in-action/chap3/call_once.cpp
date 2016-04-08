#include <thread>
#include <mutex>
#include <iostream>

class X {
private:
    int detail;
    int cnt;
    std::once_flag init_flag;

    void open_connection() {
        std::cout << __func__ << std::endl;
    }

public:
    void send_data(int val) {
        std::call_once(init_flag, &X::open_connection, this);
    }
};
