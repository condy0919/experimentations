#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

std::condition_variable cvs[4];
std::mutex mtx[4];

#define NEXT(idx) (((idx) + 1) % 4)

void foo(int id) {
    while (true) {
        std::unique_lock<std::mutex> lk(mtx[id]);
        cvs[id].wait(lk);
        std::cout << id << std::endl;
        const int next = NEXT(id);
        cvs[next].notify_one();
    }
}

int main() {
    //std::thread tds[4];
    //for (int i = 0; i < 4; ++i) {
    //    tds[i] = std::thread(foo, i);
    //}
    //cvs[0].notify_one();
    //for (int i = 0; i < 4; ++i) {
    //    tds[i].join();
    //}
    return 0;
}
