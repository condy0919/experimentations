#include <iostream>
#include <memory>
#include <utility>

template <typename T>
class queue {
private:
    struct node {
        T data;
        std::unique_ptr<node> next;

        node(T data_) : data(std::move(data_)) {}
    };

    std::unique_ptr<node> head;  // 1
    node* tail;                  // 2

public:
    queue() {}
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;

    std::shared_ptr<T> try_pop() {
        if (!head) {
            return std::shared_ptr<T>();
        }
        auto res = std::make_shared<T>(std::move(head->data));
        const std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);  // 3
        return res;
    }

    void push(T val) {
        auto p = std::make_unique<node>(std::move(val));
        node* const new_tail = p.get();
        if (tail) {
            tail->next = std::move(p);  // 4
        } else {
            head = std::move(p);  // 5
        }
        tail = new_tail;  // 6
    }
};
