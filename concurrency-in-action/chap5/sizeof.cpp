#include <iostream>
#include <string>

struct my_data {
    int i;
    double d;
    unsigned bf1 : 10;
    int bf2 : 25;
    int bf3 : 1;
    int bf4 : 9;
    int i2;
    char c1, c2;
    std::string s;
};

int main() {
    std::cout << sizeof(std::string) << std::endl;
    std::cout << sizeof(my_data) << std::endl;
    return 0;
}
