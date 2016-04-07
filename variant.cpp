#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <utility>
#include <string>
#include <iostream>

#define GENERATE(name, op)                                                \
    template <typename...>                                                \
    struct name;                                                          \
    template <typename T>                                                 \
    struct name<T> : std::integral_constant<size_t, op(T)> {};            \
    template <typename T, typename... Ts>                                 \
    struct name<T, Ts...>                                                 \
        : std::integral_constant<size_t, (op(T) > name<Ts...>::value      \
                                              ? op(T)                     \
                                              : name<Ts...>::value)> {};

GENERATE(MaxTypeSize, sizeof)
GENERATE(MaxTypeAlign, alignof)
#undef GENERATE

template <typename...>
struct Contains : std::false_type {};

template <typename T, typename First, typename... Ts>
struct Contains<T, First, Ts...>
    : std::integral_constant<bool, std::is_same<T, First>::value ||
                                       Contains<T, Ts...>::value> {};

//template <typename...>
//struct ContainsConvertibleType : std::false_type {};
//
//template <typename T, typename First, typename... Ts>
//struct ContainsConvertibleType<T, First, Ts...>
//    : std::integral_constant<
//          bool, std::is_convertible<T, First>::value ||
//                    ContainsConvertibleType<T, Ts...>::value> {};
//
//template <bool, typename, typename>
//struct IF;
//
//template <typename T, typename U>
//struct IF<true, T, U> {
//    using type = T;
//};
//
//template <typename T, typename U>
//struct IF<false, T, U> {
//    using type = U;
//};
//
//template <typename...>
//struct ConvertibleType {
//    using type = void;
//};
//
//template <typename T, typename First, typename... Tail>
//struct ConvertibleType<T, First, Tail...> {
//    using type =
//        typename IF<std::is_convertible<T, First>::value, First,
//                    typename ConvertibleType<T, Tail...>::type>::type;
//};

template <typename...>
struct VariantHelper;

template <>
struct VariantHelper<> {
    static void destory(std::type_index, void*) {}
    static void copy(std::type_index, void*, void*) {}
    static void move(std::type_index, void*, void*) {}
};

template <typename T, typename... Ts>
struct VariantHelper<T, Ts...> {
    static void destory(std::type_index ti, void* src) {
        if (ti == typeid(T))
            reinterpret_cast<T*>(src)->~T();
        else
            VariantHelper<Ts...>::destory(ti, src);
    }
    static void copy(std::type_index ti, void* src, void* dst) {
        if (ti == typeid(T))
            new (dst) T(*reinterpret_cast<T*>(src));
        else
            VariantHelper<Ts...>::copy(ti, src, dst);
    }
    static void move(std::type_index ti, void* src, void* dst) {
        if (ti == typeid(T))
            new (dst) T(std::move(*reinterpret_cast<T*>(src)));
        else
            VariantHelper<Ts...>::move(ti, src, dst);
    }
};

template <typename... Ts>
class Variant {
    typedef VariantHelper<Ts...> Helper;

public:
    Variant(const Variant& rhs) : type_index(rhs.type_index) {
        Helper::copy(type_index, &rhs.data, &data);
    }

    Variant& operator=(const Variant& rhs) {
        Helper::destory(type_index, &data);
        type_index = rhs.type_index;
        Helper::copy(type_index, &rhs.data, &data);
        return *this;
    }

    template <typename T,
              class = std::enable_if_t<Contains<T, Ts...>::value>>
    Variant(const T& value) : type_index(typeid(T)) {
        new (&data) T(value);
    }

    template <class = std::enable_if_t<Contains<std::string, Ts...>::value>>
    Variant(const char* s) : type_index(typeid(std::string)) {
        new (&data) std::string(s);
    }

    // template <typename T, class = std::enable_if_t<
    //                          ContainsConvertibleType<T, Ts...>::value>>
    // Variant(const T& value)
    //    : type_index(typeid(typename ConvertibleType<T, Ts...>::type)) {
    //    using U = typename ConvertibleType<T, Ts...>::type;
    //    new (&data) U(value);
    //}

    Variant(Variant&& rhs) noexcept
        : type_index(std::move(rhs.type_index)) {
        Helper::move(type_index, &rhs.data, &data);
        rhs.type_index = typeid(void);
    }

    Variant& operator=(Variant&& rhs) noexcept {
        Helper::destory(type_index, &data);
        type_index = std::move(rhs.type_index);
        Helper::move(type_index, &rhs.data, &data);
        rhs.type_index = typeid(void);
        return *this;
    }

    ~Variant() { Helper::destory(type_index, &data); }

    template <typename T>
    bool is() const {
        return type_index == typeid(T);
    }

    template <typename T>
    const T& get() const {
        if (!is<T>())
            throw std::bad_cast();
        return *(T*)&data;
    }

private:
    std::aligned_storage_t<MaxTypeSize<Ts...>::value,
                           MaxTypeAlign<Ts...>::value> data;
    std::type_index type_index;
};

int main() {
    typedef Variant<int, std::string, double> cv;

    cv a = 10;
    std::cout << a.get<int>() << std::endl;

    a = "hello";
    //a = std::string("hello");
    std::cout << a.get<std::string>() << std::endl;

    a = 3.1415926;
    std::cout << a.get<double>() << std::endl;

    return 0;
}
