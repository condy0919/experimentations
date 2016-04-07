#include <iostream>
#include <algorithm>
#include <utility>

using namespace std;

template <typename Inner, typename Outer>
class composed {
public:
    explicit composed(Inner inner = Inner(), Outer outer = Outer())
        : _inner(inner), _outer(outer) {}

    template <typename... T>
    auto operator()(T... x) const {
        return _outer(_inner(x...));
    }

private:
    Inner _inner;
    Outer _outer;
};

template <typename F1, typename F2>
constexpr composed<F1, F2> compose(F1 f1, F2 f2) {
    return composed<F1, F2>(f1, f2);
}

template <typename T, typename... Rest>
constexpr auto compose(T f1, Rest... f) {
    return composed<T, decltype(compose(f...))>(f1, compose(f...));
}

int f(int x) {
    return ++x;
}

int g(int x) {
    return 2 * x;
}

int h(int x, int y) {
    return x + y;
}

template <size_t N>
struct repeat {
    template <typename Fn>
    constexpr static auto generate(Fn f) {
        return compose(f, repeat<N - 1>::generate(f));
    }
};

template <>
struct repeat<1> {
    template <typename Fn>
    constexpr static auto generate(Fn f) {
        return f;
    }
};

int main() {
    // cout << compose(f, g, g, g, f)(2) << endl;  // 25 expected
    // cout << compose([](int x) { return x + 1; }, [](int x) { return 2 * x;
    // })(2) << endl;
    cout << repeat<5>::generate(f)(0) << endl;
    cout << compose(h, g, g, g, f)(2, 3) << endl; // 41 expected

    return 0;
}
