
#include <iostream>

#include "static_string.hpp"

int main() {
    using hello = STATIC_STRING("hello world");
    std::cerr << hello() << std::endl;
    //std::cerr << "hello" << std::endl;
}
