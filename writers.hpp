#include <type_traits>
#include <algorithm>
#include <cstring>

#include "meta.hpp"

template<class Int, EnableIfQ< std::is_signed<Int> > = enable>
char* write_decimal(char* s, Int n) {
    Int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    std::reverse(s, s+i);
    return s+i;
}

template<class UInt, EnableIfQ< std::is_unsigned<UInt> > = enable>
char* write_decimal(char* s, UInt n) {
    UInt i = 0U;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    std::reverse(s, s+i);
    return s+i;
}

inline char* write_decimal(char* s, double dl) {
    return write_decimal(s, static_cast<long long>(dl));    
}

inline char* write_decimal(char* s, float dl) {
    return write_decimal(s, static_cast<long long>(dl));    
}

template<class Int, EnableIfQ< std::is_integral<Int> > = enable>
char* write_hexadecimal(char* s, Int input) {
    using UInt = typename std::make_unsigned<Int>::type;
    constexpr auto table = "0123456789abcdef";
    UInt n = static_cast<UInt>(input);
    UInt mask = 0X0F;
    UInt i = 0U;
    do {       /* generate digits in reverse order */
        s[i++] = table[n & mask];   /* get next digit */
    } while ((n >>= 4) != 0);     /* delete it */
    std::reverse(s, s+i);
    return s+i;
}

template<class Int, EnableIfQ< std::is_integral<Int> > = enable>
char* write_HEXadecimal(char* s, Int input) {
    using UInt = typename std::make_unsigned<Int>::type;
    constexpr auto table = "0123456789ABCDEF";
    UInt n = static_cast<UInt>(input);
    UInt mask = 0X0F;
    UInt i = 0U;
    do {       /* generate digits in reverse order */
        s[i++] = table[n & mask];   /* get next digit */
    } while ((n >>= 4) != 0);     /* delete it */
    std::reverse(s, s+i);
    return s+i;
}

template<class Int, EnableIfQ< std::is_integral<Int> > = enable>
char* write_octal(char* s, Int input) {
    using UInt = typename std::make_unsigned<Int>::type;
    UInt n = static_cast<UInt>(input);
    UInt mask = 007;
    UInt i = 0U;
    do {       /* generate digits in reverse order */
        s[i++] = (n & mask) + '0';   /* get next digit */
    } while ((n >>= 3) != 0);     /* delete it */
    std::reverse(s, s+i);
    return s+i;
}

template<class Int, EnableIfQ< std::is_integral<Int> > = enable> 
char* write_char(char* s, Int input) {
    *s = static_cast<char>(input);
    return s + 1;
}

char* write_string(char* s, const char* what) {
    auto len = std::strlen(what);
    std::memcpy(s, what, len);
    return s + len;
}

char* write_string(char* s, char what) {
    return write_char(s, what);
}

template<class Int, EnableIfQ< std::is_integral<Int> > = enable>
char* write_string(char* s, Int n) {
    return write_decimal(s, n);
}