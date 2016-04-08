#include <future>
#include <cmath>
#include <stdexcept>
#include <iostream>

double square_root(double x) {
    if (x < 0)
        throw std::out_of_range("x < 0");
    return std::sqrt(x);
}

int main() {
    auto f1 = std::async(square_root, -1);
    std::cout << f1.get() << std::endl;
    return 0;
}
