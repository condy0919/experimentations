#include <exception>
#include <algorithm>
#include <mutex>
#include <memory>
#include <stack>

struct empty_stack : std::exception {
    const char* what() const throw() {
        return "empty stack";
    }
};

template <typename T>
class threadsafe_stack {
private:
    std::stack<T> data;
    mutable std::mutex m;

public:
    threadsafe_stack() {}
    threadsafe_stack(const threadsafe_stack& rhs) {
        std::lock_guard<std::mutex> guard(rhs.m);
        data = rhs.data;
    }

    threadsafe_stack& operator=(const threadsafe_stack& rhs) = delete;
    void push(T val) {
        std::lock_guard<std::mutex> guard(m);
        data.push(std::move(val));
    }

    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> guard(m);
        if (data.empty())
            throw empty_stack();
        auto res = std::make_shared<T>(std::move(data.top()));
        data.pop();
        return res;
    }

    void pop(T& val) {
        std::lock_guard<std::mutex> guard(m);
        if (data.empty())
            throw empty_stack();
        val = std::move(data.top());
        data.pop();
    }

    bool empty() const {
        std::lock_guard<std::mutex> guard(m);
        return data.empty();
    }
};
