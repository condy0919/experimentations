#include <iostream>
#include <algorithm>
#include <list>
#include <future>

template <typename T>
std::list<T> sequential_quick_sort(std::list<T> input) {
    if (input.empty())
        return input;

    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    const T& pivot = *result.begin();

    auto pts = std::partition(input.begin(), input.end(),
                              [pivot](const T& x) { return x < pivot; });
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), pts);
    auto new_lower = sequential_quick_sort(std::move(lower_part)),
         new_higher = sequential_quick_sort(std::move(input));
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}

template <typename T>
std::list<T> parallel_quick_sort(std::list<T> input) {
    if (input.empty())
        return input;

    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    const T& pivot = *result.begin();

    auto pt = std::partition(input.begin(), input.end(),
                             [&](const T& x) { return x < pivot; });
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), pt);

    std::future<std::list<T>> new_lower(
        std::async(&parallel_quick_sort<T>, std::move(lower_part)));

    auto new_higher = parallel_quick_sort(std::move(input));
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get());
    return result;
}

int main() {
    std::list<int> lst = {4, 1, 2, 3, 5, 6, 7, 8};
    lst = parallel_quick_sort(lst);
    for (auto&& c : lst) {
        std::cout << c << ' ';
    }
    std::cout << std::endl;
    return 0;
}
