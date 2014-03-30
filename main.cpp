
#include <iostream>
#include <vector>

#include "crazy_printf.hpp"

#define SPRINTF(WHERE, LIT, ...) vsprintf<STATIC_STRING(LIT)>((WHERE), __VA_ARGS__)

int main() {
    using hello = STATIC_STRING("hello world");
    std::cerr << hello() << std::endl;

    std::vector<char> vec(2048);
    SPRINTF(vec.data(), "%d%d%x%o%s %s!! %%%d%%%s  %c", 22, ~22, ~22, ~22, "~Hi!!", "Wrld", 3.14, 0xf, 0x22);

    std::cerr << vec.data() << std::endl;
    //std::cerr << "hello" << std::endl;
}
