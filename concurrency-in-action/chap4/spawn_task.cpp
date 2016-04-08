#include <iostream>
#include <algorithm>
#include <functional>
#include <thread>
#include <future>

template <typename F, typename Arg>
std::future<typename std::result_of<F(Arg&&)>::type> spawn_task(F&& f, Arg&& arg) {
    typedef typename std::result_of<F(Arg&&)>::type result_type;
    std::packaged_task<result_type(Arg&&)> task(std::move(f));
    std::future<result_type> res = task.get_future();
    std::thread t(std::move(task), std::move(arg));
    t.detach();
    return res;
}
