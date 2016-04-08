#include <iostream>

using namespace std;

#define CACHE_LINE_SIZE 64
#define ROUNDUP(x) ((x + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1))

struct some_struct {
    int a;
    char b;
};

template <typename T>
struct align_cache_line : T {
    char __padding[ROUNDUP(sizeof(T)) - sizeof(T)];
};

alignas(128) char c[2];

int main() {
    cout << sizeof(align_cache_line<some_struct>) << endl;
    //cout << sizeof(align_cache_line<int>) << endl;
    return 0;
}
