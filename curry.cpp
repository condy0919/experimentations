#include <iostream>
#include <functional>
#include <type_traits>
#include <cassert>

/*
 * auto f = curry(foo);   void foo(int, int, int) {}
 * f(1)(2)(3);
 * f(1,2)(3);
 * f(1,2,3);
 * f(1) returns a lambda taking 2 parameters.
 * f(1,2) returns a lambda taking 1 parameter.
 * f(1,2,3) returns a value.
 */

namespace lambda {
namespace detail {
template <typename R, typename TupleType, size_t... I>
auto helper(std::index_sequence<I...>) {
    using t = std::function<R(std::tuple_element_t<I, TupleType>...)>;
    return t();
}

template <size_t N>
struct drop {
    template <typename First, typename... Args>
    struct elems {
        using type = typename drop<N - 1>::template elems<Args...>::type;
    };
};

template <>
struct drop<0> {
    template <typename... Args>
    struct elems {
        using type = std::tuple<Args...>;
    };
};

template <typename>
class CurryObject;

template <typename R, typename... Args>
class CurryObject<R(Args...)> : public CurryObject<std::function<R(Args...)>> {
public:
    CurryObject(R f(Args...))
        : CurryObject<std::function<R(Args...)>>(std::move(f)) {}
};

template <typename R, typename... Args>
class CurryObject<R (*)(Args...)> : public CurryObject<R(Args...)> {
public:
    CurryObject(R (*pf)(Args...)) : CurryObject<R(Args...)>(pf) {}
};

template <typename R, typename... Args>
class CurryObject<std::function<R(Args...)>> {
public:
    CurryObject(std::function<R(Args...)> f) : fn(std::move(f)) {}

    // full apply
    template <typename... Params,
              std::enable_if_t<sizeof...(Params) == sizeof...(Args), int> = 0>
    auto operator()(Params&&... parms) {
        return fn(std::forward<Params>(parms)...);
    }

    // partial apply
    template <typename... Params,
              std::enable_if_t<sizeof...(Params) < sizeof...(Args), int> = 0>
    auto operator()(Params&&... parms) {
        using tuple_type =
            typename drop<sizeof...(Params)>::template elems<Args...>::type;
        using func_type = decltype(helper<R, tuple_type>(
            std::make_index_sequence<sizeof...(Args) - sizeof...(Params)>{}));

        func_type f = [&](auto... xs) {
            return fn(std::forward<Params>(parms)..., xs...);
        };
        return CurryObject<func_type>(f);
    }

private:
    std::function<R(Args...)> fn;
};

template <typename T>
struct memfun_type {
    using type = void;
};

template <typename R, typename C, typename... Args>
struct memfun_type<R (C::*)(Args...) const> {
    using type = std::function<R(Args...)>;
};
} // namespace detail

template <typename F, std::enable_if_t<!std::is_class<F>::value, int> = 0>
auto curry(F f) {
    return detail::CurryObject<F>(f);
}

template <typename F, std::enable_if_t<std::is_class<F>::value, int> = 0>
auto curry(F f) {
    using t = typename detail::memfun_type<decltype(&F::operator())>::type;
    return detail::CurryObject<t>(f);
}
} // namespace lambda

int foo(int a, int b, int c) {
    return a + b + c;
}

struct bar {
    void operator()() {
        std::cout << __PRETTY_FUNCTION__ << '\n';
    }
};

int main() {
    using namespace lambda;

    auto f = curry(foo);
    auto g = curry([](int x, int y, int z) { return x * y + z; });

    assert(f(1, 2, 4) == 7);
    assert(f(1, 4)(10) == 15);
    assert(f(1)(8)(64) == 73);
    assert(g(1)(2)(3) == 5);
    assert(g(1, 3)(4) == 7);
    assert(g(2, 5, 7) == 17);
}
