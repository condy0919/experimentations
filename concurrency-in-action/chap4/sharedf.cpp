#include <future>
#include <iostream>
#include <algorithm>
#include <thread>
#include <functional>
#include <cassert>

int main() {
    std::promise<int> p;
    std::future<int> f = p.get_future();
    assert(f.valid());
    std::shared_future<int> sf(std::move(f));
    assert(!f.valid());
    assert(sf.valid());
    return 0;
}
