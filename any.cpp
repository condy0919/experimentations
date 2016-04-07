#include <typeinfo>
#include <algorithm>
#include <iostream>
#include <string>

class any {
public:
    class placeholder {
    public:
        virtual ~placeholder() {}
        virtual const std::type_info& type() const = 0;
        virtual placeholder* clone() const = 0;
    };

    template <typename value_type>
    class holder : public placeholder {
    public:
        holder(const value_type& value) : held(value) {}
        virtual const std::type_info& type() const {
            return typeid(value_type);
        }
        virtual placeholder* clone() const {
            return new holder(held);
        }
        value_type held;
    };

    placeholder* content;

    template <typename value_type>
    any(const value_type& value)
        : content(new holder<value_type>(value)) {}

    any(const any& rhs) : content(rhs.content ? rhs.content->clone() : 0) {}

    template <typename value_type>
    any& operator=(const value_type& rhs) {
        any(rhs).swap(*this);
        return *this;
    }

    any& swap(any& rhs) {
        std::swap(content, rhs.content);
        return *this;
    }

    const std::type_info& type() const {
        return content->type();
    }

    ~any() {
        delete content;
    }
};

template <typename value_type>
value_type* any_cast(const any* operand) {
    return operand && (operand->type() == typeid(value_type))
               ? &static_cast<any::holder<value_type>*>(operand->content)->held
               : 0;
}

template <typename value_type>
value_type any_cast(const any& operand) {
    const value_type* result = any_cast<value_type>(&operand);
    if (!result)
        throw std::bad_cast();
    return *result;
}

int main() {
    any i = 10;
    any s = std::string("i'm string");
    std::cout << any_cast<int>(i) << std::endl;
    std::cout << any_cast<std::string>(s) << std::endl;
}
