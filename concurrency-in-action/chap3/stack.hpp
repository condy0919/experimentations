#include <exception>
#include <memory>
#include <mutex>
#include <stack>

struct empty_stack : public std::exception {
    const char* what() const throw() {
        return "empty stack";
    }
};

template <typename T>
class thread_stack {
private:
    std::stack<T> data;
    mutable std::mutex m;

public:
    thread_stack() {}
    thread_stack(const thread_stack& rhs) {
        std::lock_guard<std::mutex> guard(rhs.m);
        data = rhs.data;
    }
    thread_stack& operator=(const thread_stack& rhs) = delete;

    void push(T value) {
        std::lock_guard<std::mutex> guard(m);
        data.push(value);
    }

    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> guard(m);
        if (data.empty())
            throw empty_stack();
        const std::shared_ptr<T> res(std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }

    void pop(T& value) {
        std::lock_guard<std::mutex> guard(m);
        if (data.empty())
            throw empty_stack();
        value = data.top();
        data.pop();
    }

    bool empty() const {
        std::lock_guard<std::mutex> guard(m);
        return data.empty();
    }
};
