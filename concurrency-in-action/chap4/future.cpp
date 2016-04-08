#include <future>
#include <iostream>

int find_the_answer_to_ltuae() {
    int s = 0;
    for (int i = 0; i < 10000; ++i) {
        s += i;
    }
    return s;
}

void do_other_stuff() {
    std::cout << __PRETTY_FUNCTION__ << '\n';
}

int main() {
    std::future<int> ans = std::async(find_the_answer_to_ltuae);
    do_other_stuff();
    std::cout << "The answer is " << ans.get() << std::endl;
    return 0;
}
