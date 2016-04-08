#include <thread>
#include <iostream>
#include <functional>
#include <algorithm>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <list>

using namespace std;

std::list<int> some_list;
std::mutex some_mutex;

void add_to_list(int new_value) {
    std::lock_guard<std::mutex> guard(some_mutex);
    some_list.push_back(new_value);
}

bool list_contains(int value) {
    std::lock_guard<std::mutex> guard(some_mutex);
    return std::find(some_list.begin(), some_list.end(), value) !=
           some_list.end();
}
