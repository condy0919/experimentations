#include <iostream>
#include <algorithm>
#include <memory>

template <typename T>
class queue {
private:
    struct node {
        std::shared_ptr<T> data;  // 1
        std::unique_ptr<node> next;
    };

    std::unique_ptr<node> head;
    node* tail;

public:
    queue() : head(new node), tail(head.get()) {}  // 2
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;

    std::shared_ptr<T> try_pop() {
        if (head.get() == tail) {  // 3
            return std::shared_ptr<T>();
        }
        const std::shared_ptr<T> res(head->data);  // 4
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);  // 5
        return res;                        // 6
    }

    void push(T val) {
        auto new_data = std::make_shared<T>(std::move(val));  // 7
        std::unique_ptr<node> p(new node);                    // 8
        tail->data = new_data;                                // 9
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
};
