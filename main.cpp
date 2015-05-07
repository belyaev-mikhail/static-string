
#include <iostream>
#include <vector>
#include <string>

#include "crazy_printf.hpp"

#include "crazy_lisper.hpp"

template<class T>
std::string typesig() {
    return __PRETTY_FUNCTION__;
}

#define SPRINTF(WHERE, LIT, ...) vsprintf<STATIC_STRING(LIT)>((WHERE), __VA_ARGS__)

#define _SS(FMT) STATIC_STRING(FMT)()

#define LISP(...) lispIt< STATIC_STRING(#__VA_ARGS__) >

#define EVAL(...) resolve_type< LISP(__VA_ARGS__) >

int main() {
    using hello = STATIC_STRING("hello world");
    std::cerr << hello() << std::endl;

    std::vector<char> vec(2048);
    vsprintf(vec.data(), _SS("%d%d%x%o%s%s!! %%%d %%%s  %c"), 22, ~22, ~22, ~22, "~Hi!!", "Wrld", 3.14, 0xf, 0x22);

    std::cerr << vec.data() << std::endl;

    std::cerr << typesig< lispIt< STATIC_STRING(" a b ('()c d e) ff aa   ") > >() << std::endl;
    std::cerr << typesig< lispIt< STATIC_STRING(" a") > >() << std::endl;
    std::cerr << isSpecial<'\0'>() << std::endl;

    std::cerr << typesig< lispIt< STATIC_STRING("(aaaa ( bbbb cccc dddd ) abcde () ) xxxx ") > >() << std::endl;
    //std::cerr << "hello" << std::endl;

    NAMED_TYPE("int") i = 52;

    std::cerr << typesig< NAMED_TYPE("int") >() << std::endl;

    std::cerr << typesig< EVAL(tuple int ( tuple char char ) double) > () << std::endl;
    std::cerr << typesig< EVAL(tuple int ( pair char char ) double) > () << std::endl;
    std::cerr << typesig< EVAL(+ 2 455 4 (- 16) (- 4 2)) > () << std::endl;
    std::cerr << typesig< EVAL(== 2 (- 4 2) (+ 1 1) (- (- 2))) > () << std::endl;
    std::cerr << typesig< EVAL(== 3 (- 4 2)) > () << std::endl;

    std::cerr << typesig< EVAL(== int int32_t) > () << std::endl;
    std::cerr << EVAL(tuple_size (pair int char))::value << std::endl;
    std::cerr << typesig< EVAL(
        ite (== int int32_t) uint32_t (+ char)
    ) > () << std::endl;


    std::cerr << typesig< EVAL(id int) > () << std::endl;

    std::cerr << typesig< EVAL(== (typeof int) type) > () << std::endl;
}
