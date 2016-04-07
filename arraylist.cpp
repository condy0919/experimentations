#include <vector>
#include <iostream>
#include <algorithm>
#include <exception>

template <typename T>
class ArrayList : public std::vector<T> {
public:
    class Proxy {
    public:
        Proxy(ArrayList* al, std::size_t pos)
            : pv(static_cast<std::vector<T>*>(al)), pos(pos) {}
        Proxy& operator=(T v) {
            if (pos >= pv->size())
                pv->resize(pos + 1);
            pv->operator[](pos) = v;
            return *this;
        }
        operator T() const {
            if (pos >= pv->size())
                throw std::exception();
            return pv->operator[](pos);
        }

    private:
        std::vector<T>* pv;
        std::size_t pos;
    };

    using std::vector<T>::vector;
    Proxy operator[](std::size_t pos) { return Proxy(this, pos); }
};

int main() {
    ArrayList<int> a = {1, 2, 3};
    std::fill(a.begin(), a.end(), 0xff);
    std::for_each(a.begin(), a.end(),
                  [](const auto& x) { std::cout << x << '\t'; });
    std::cout << '\n';
}
