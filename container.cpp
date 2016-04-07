#include <iostream>
#include <tuple>
#include <memory>
#include <type_traits>
#include <boost/any.hpp>

using namespace std;

template <typename T, typename = enable_if_t<!std::is_function<T>::value>>
struct closure_traits : public closure_traits<decltype(&T::operator())> {};

template <typename R, typename... Args>
struct closure_traits<R (*)(Args...)> {
    static constexpr const size_t arity = sizeof...(Args);

    using ReturnType = R;
    using PackedParameterType = std::tuple<Args...>;
    template <size_t i>
    struct arg {
        using type = typename tuple_element<i, PackedParameterType>::type;
    };
};

template <typename Class, typename R, typename... Args>
struct closure_traits<R (Class::*)(Args...) const> {
    static constexpr const size_t arity = sizeof...(Args);

    using ReturnType = R;
    using PackedParameterType = std::tuple<Args...>;
    template <size_t i>
    struct arg {
        using type = typename tuple_element<i, PackedParameterType>::type;
    };
};

template <typename Func, typename Tup, size_t... index>
auto invoke_helper(Func&& f, Tup&& tup, std::index_sequence<index...>) {
    return f(std::get<index>(std::forward<Tup>(tup))...);
}

template <typename Func, typename Tup>
auto invoke(Func&& f, Tup&& tup) {
    constexpr size_t sz = std::tuple_size<std::decay_t<Tup>>::value;
    return invoke_helper(std::forward<Func>(f), std::forward<Tup>(tup),
                         std::make_index_sequence<sz>());
}

class LambdaContainer {
private:
    struct helper_base {
        virtual void remove(void*) = 0;
        virtual boost::any call(void*) = 0;
        virtual void reset(void*) = 0;
    };

    template <typename T>
    struct helper : public helper_base {
        using PackedParameterType =
            typename closure_traits<T>::PackedParameterType;

        struct parameter {
            parameter() = default;
            parameter(PackedParameterType arg) : paras(std::move(arg)) {}
            boost::any apply(void* objptr) {
                return invoke(*(T*)objptr, paras);
            }

        private:
            PackedParameterType paras;
        };

        helper() = default;

        virtual void remove(void* p) {
            delete (T*)p;
        }

        virtual boost::any call(void* p) {
            return pa.apply(p);
        }

        virtual void reset(void* tuple_ptr) {
            pa = parameter(*(PackedParameterType*)tuple_ptr);
        }

    private:
        parameter pa;
    };

    void* objptr;
    std::shared_ptr<helper_base> caller;

public:
    template <typename T>
    LambdaContainer(T functor) {
        objptr = (void*)(new T(std::move(functor)));
        caller = make_shared<helper<T>>();
    }

    template <typename... Args>
    boost::any operator()(Args... args) {
        auto tp = std::tuple<Args...>(args...);
        caller->reset((void*)&tp);
        return caller->call(objptr);
    }

    ~LambdaContainer() {
        caller->remove(objptr);
    }
};

boost::any add_fn(int a, int b) {
    return a + b;
}

int main() {
    LambdaContainer plus1 = [](int x) -> boost::any { return x + 1; };
    LambdaContainer doubleme = [](double x) -> boost::any { return x * 2; };
    LambdaContainer add = add_fn;

    boost::any rt = plus1(1);
    cout << boost::any_cast<int>(rt) << endl;

    rt = doubleme(3.2);
    cout << boost::any_cast<double>(rt) << endl;

    rt = add(1, 2);
    cout << boost::any_cast<int>(rt) << endl;

    rt = add(100);
    cout << boost::any_cast<int>(rt) << endl;

    return 0;
}
