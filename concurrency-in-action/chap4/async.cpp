#include <iostream>
#include <string>
#include <future>

struct X {
    void foo(int idx, const std::string& s) {
        std::cout << idx << ", " << s << std::endl;
    }
    std::string bar(const std::string& s) {
        return s;
    }
};

X x;
auto f1 = std::async(&X::foo, &x, 42, "hello");
auto f2 = std::async(&X::bar, x, "goodbye");

struct Y {
    double operator()(double d) {
        return 2 * d;
    }
};

Y y;
auto f3 = std::async(Y(), 3.14);
auto f4 = std::async(std::ref(y), 2.718);

X baz(X& x) {
    return X(x);
}

//std::async(baz, std::ref(x));
class move_only {
public:
    move_only() = default;
    move_only(move_only&&) {}
    move_only(const move_only&) = delete;
    move_only& operator=(move_only&&);
    move_only& operator=(const move_only&) = delete;

    void operator()(){}
};
auto f5 = std::async(move_only());

int main() {
    return 0;
}
