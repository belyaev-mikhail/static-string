
#include "static_string.hpp"

#include <algorithm>
#include <cstring>

template<class Int>
inline 
typename std::enable_if<std::is_signed<Int>::value, char*>::type
write_decimal(char* s, Int n) {
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

template<class UInt>
inline 
typename std::enable_if<std::is_unsigned<UInt>::value, char*>::type
write_decimal(char* s, UInt n) {
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

template<class Int>
inline 
typename std::enable_if<std::is_integral<Int>::value, char*>::type
write_hexadecimal(char* s, Int input) {
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

template<class Int>
inline 
typename std::enable_if<std::is_integral<Int>::value, char*>::type
write_HEXadecimal(char* s, Int input) {
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

template<class Int>
inline 
typename std::enable_if<std::is_integral<Int>::value, char*>::type
write_octal(char* s, Int input) {
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

template<class Int>
inline 
typename std::enable_if<std::is_integral<Int>::value, char*>::type
write_char(char* s, Int input) {
    *s = static_cast<char>(input);
    return s + 1;
}

inline char* write_string(char* s, const char* what) {
    auto len = std::strlen(what);
    std::memcpy(s, what, len);
    return s + len;
}

inline char* write_string(char* s, char what) {
    return write_char(s, what);
}

template<class Int>
inline 
typename std::enable_if<std::is_integral<Int>::value, char*>::type
write_string(char* s, Int n) {
    return write_decimal(s, n);
}

template<class ...Elems> struct type_list{};

template<class H, class T> struct tl_cons;
template<class H, class ...Elems> 
struct tl_cons<H, type_list<Elems...>>
{
    using type = type_list<H, Elems...>;
};
template<class H, class TL> using tl_cons_t = typename tl_cons<H, TL>::type;

struct error_too_many_arguments{};
struct error_too_little_arguments{};
struct error_illegal_format_string{};

template<class Arg> struct format_decimal{};
template<class Arg> struct format_octal{};
template<class Arg> struct format_hexadecimal{};
template<class Arg> struct format_HEXadecimal{};
template<class Arg> struct format_string{};
template<class Arg> struct format_char{};

template<class SS, class... Args> struct PPrintf;
template<class Arg, char... Ss> struct PFormat;

template<>
struct PPrintf< static_string<> > {
    using ret = type_list<>;
};

template<class ...Args>
struct PPrintf< static_string<>, Args... > {
    static_assert(!sizeof...(Args), "Too many arguments to printf!");

    using ret = type_list<>;
};

template<char ...Rest>
struct PPrintf< static_string< '%', '%', Rest...> >{
    using ret = tl_cons_t< static_string<'%'>, typename PPrintf< static_string<Rest...> >::ret >;
};
// this case is just to resolve ambiguity
template<char ...Rest, class Arg, class ...Args>
struct PPrintf< static_string< '%', '%', Rest...>, Arg, Args... >{
    using ret = tl_cons_t< static_string<'%'>, typename PPrintf< static_string<Rest...>, Arg, Args... >::ret >;
};

template<char ...Rest>
struct PPrintf< static_string< '%', Rest...> > {
    static_assert(!sizeof...(Rest), "Too little arguments to printf!");

    using ret = type_list<>;
};

template<char ...Rest, class Arg, class ...Args>
struct PPrintf< static_string< '%', Rest...>, Arg, Args... > {
    using formatted = PFormat< Arg, Rest... >;
    using tail = typename formatted::tail;
    using res = typename formatted::res;

    using ret = tl_cons_t< res, typename PPrintf< tail, Args...>::ret >; 
};

template<char H, char ...Rest, class ...Args>
struct PPrintf< static_string< H, Rest...>, Args... > {
    using ret = tl_cons_t< static_string< H >, typename PPrintf< static_string<Rest...>, Args... >::ret >;
};

template< class Arg, char...Rest>
struct PFormat // default case
{
    static_assert(!sizeof...(Rest), "Illegal format!");
    using tail = static_string< Rest ... >;
    using res = format_string<Arg>;
};

template< class Arg, char...Rest>
struct PFormat<Arg, 'd', Rest...>
{
    using tail = static_string< Rest ... >;
    using res = format_decimal<Arg>;
};

template< class Arg, char...Rest>
struct PFormat<Arg, 'i', Rest...>
{
    using tail = static_string< Rest ... >;
    using res = format_decimal<Arg>;
};

template< class Arg, char...Rest>
struct PFormat<Arg, 'x', Rest...>
{
    using tail = static_string< Rest ... >;
    using res = format_hexadecimal<Arg>;
};

template< class Arg, char...Rest>
struct PFormat<Arg, 'X', Rest...>
{
    using tail = static_string< Rest ... >;
    using res = format_HEXadecimal<Arg>;
};

template< class Arg, char...Rest>
struct PFormat<Arg, 'o', Rest...>
{
    using tail = static_string< Rest ... >;
    using res = format_octal<Arg>;
};

template< class Arg, char...Rest>
struct PFormat<Arg, 'c', Rest...>
{
    using tail = static_string< Rest ... >;
    using res = format_char<Arg>;
};

template< class Arg, char...Rest>
struct PFormat<Arg, 's', Rest...>
{
    using tail = static_string< Rest ... >;
    using res = format_string<Arg>;
};

template<class Arg>
char* pprocess(format_string<Arg>, char * buf, Arg&& arg) {
    return write_string(buf, std::forward<Arg>(arg));
}

template<class Arg>
char* pprocess(format_decimal<Arg>, char * buf, Arg&& arg) {
    return write_decimal(buf, std::forward<Arg>(arg));
}

template<class Arg>
char* pprocess(format_hexadecimal<Arg>, char * buf, Arg&& arg) {
    return write_hexadecimal(buf, std::forward<Arg>(arg));
}

template<class Arg>
char* pprocess(format_HEXadecimal<Arg>, char * buf, Arg&& arg) {
    return write_HEXadecimal(buf, std::forward<Arg>(arg));
}

template<class Arg>
char* pprocess(format_octal<Arg>, char * buf, Arg&& arg) {
    return write_octal(buf, std::forward<Arg>(arg));
}

template<class Arg>
char* pprocess(format_char<Arg>, char * buf, Arg&& arg) {
    return write_char(buf, std::forward<Arg>(arg));
}

template<class TL> struct PProcess;
template<>
struct PProcess<type_list<>> {
    template<class ...Args>
    static char* doit(char * buf, Args&&... args) {
        *buf = '\0';
        return buf+1;
    }
};
template<char C, class ...Processors>
struct PProcess<type_list<static_string<C>, Processors...>> {
    template<class ...Args>
    static char* doit(char * buf, Args&&... args) {
        *buf = C;
        return PProcess<type_list<Processors...>>::doit(buf + 1, std::forward<Args>(args)...);
    }
};
template<class FirstProcessor, class ...Processors>
struct PProcess<type_list<FirstProcessor, Processors...>> {
    template<class FirstArg, class ...Args>
    static char* doit(char * buf, FirstArg&& firstArg, Args&&... args) {
        auto pp = pprocess(FirstProcessor{}, buf, std::forward<FirstArg>(firstArg));
        return PProcess<type_list<Processors...>>::doit(pp, std::forward<Args>(args)...);
    }
};

template<class SS, class ...Args>
char* vsprintf(char * buf, Args&&... args) {
    using FMT = typename PPrintf< SS, Args...>::ret;
    return PProcess<FMT>::doit(buf, std::forward<Args>(args)...);
}


// template<class SS, class ...Args>
// int vsprintf(char *buf)
// {
//     const char* fmt = SS{};

//     int len;
//     unsigned long num;
//     int i, base;
//     char *str;
//     const char *s;

//     int flags;              /* flags to number() */

//     int field_width;        /* width of output field */
//     int precision;          /* min. # of digits for integers; max
//                                number of chars for from string */
//     int qualifier;          /* 'h', 'l', or 'L' for integer fields */

//     for (str = buf; *fmt; ++fmt) {
//             if (*fmt != '%') {
//                     *str++ = *fmt;
//                     continue;
//             }

//             /* process flags */
//             flags = 0;
//           repeat:
//             ++fmt;          /* this also skips first '%' */
//             switch (*fmt) {
//             case '-':
//                     flags |= LEFT;
//                     goto repeat;
//             case '+':
//                     flags |= PLUS;
//                     goto repeat;
//             case ' ':
//                     flags |= SPACE;
//                     goto repeat;
//             case '#':
//                     flags |= SPECIAL;
//                     goto repeat;
//             case '':
//                     flags |= ZEROPAD;
//                     goto repeat;
//             }

//             /* get field width */
//             field_width = -1;
//             if (isdigit(*fmt))
//                     field_width = skip_atoi(&fmt);
//             else if (*fmt == '*') {
//                     ++fmt;
//                     /* it's the next argument */
//                     field_width = va_arg(args, int);
//                     if (field_width < 0) {
//                             field_width = -field_width;
//                             flags |= LEFT;
//                     }
//             }

//             /* get the precision */
//             precision = -1;
//             if (*fmt == '.') {
//                     ++fmt;
//                     if (isdigit(*fmt))
//                             precision = skip_atoi(&fmt);
//                     else if (*fmt == '*') {
//                             ++fmt;
//                             /* it's the next argument */
//                             precision = va_arg(args, int);
//                     }
//                     if (precision < 0)
//                             precision = 0;
//             }

//             /* get the conversion qualifier */
//             qualifier = -1;
//             if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
//                     qualifier = *fmt;
//                     ++fmt;
//             }

//             /* default base */
//             base = 10;

//             switch (*fmt) {
//             case 'c':
//                     if (!(flags & LEFT))
//                             while (--field_width > 0)
//                                     *str++ = ' ';
//                     *str++ = (unsigned char)va_arg(args, int);
//                     while (--field_width > 0)
//                             *str++ = ' ';
//                     continue;

//             case 's':
//                     s = va_arg(args, char *);
//                     len = strnlen(s, precision);

//                     if (!(flags & LEFT))
//                             while (len < field_width--)
//                                     *str++ = ' ';
//                     for (i = 0; i < len; ++i)
//                             *str++ = *s++;
//                     while (len < field_width--)
//                             *str++ = ' ';
//                     continue;

//             case 'p':
//                     if (field_width == -1) {
//                             field_width = 2 * sizeof(void *);
//                             flags |= ZEROPAD;
//                     }
//                     str = number(str,
//                                  (unsigned long)va_arg(args, void *), 16,
//                                  field_width, precision, flags);
//                     continue;

//             case 'n':
//                     if (qualifier == 'l') {
//                             long *ip = va_arg(args, long *);
//                             *ip = (str - buf);
//                     } else {
//                             int *ip = va_arg(args, int *);
//                             *ip = (str - buf);
//                     }
//                     continue;

//             case '%':
//                     *str++ = '%';
//                     continue;

//                     /* integer number formats - set up the flags and "break" */
//             case 'o':
//                     base = 8;
//                     break;

//             case 'x':
//                     flags |= SMALL;
//             case 'X':
//                     base = 16;
//                     break;

//             case 'd':
//             case 'i':
//                     flags |= SIGN;
//             case 'u':
//                     break;

//             default:
//                     *str++ = '%';
//                     if (*fmt)
//                             *str++ = *fmt;
//                     else
//                             --fmt;
//                     continue;
//             }
//             if (qualifier == 'l')
//                     num = va_arg(args, unsigned long);
//             else if (qualifier == 'h') {
//                     num = (unsigned short)va_arg(args, int);
//                     if (flags & SIGN)
//                             num = (short)num;
//             } else if (flags & SIGN)
//                     num = va_arg(args, int);
//             else
//                     num = va_arg(args, unsigned int);
//             str = number(str, num, base, field_width, precision, flags);
//     }
//     *str = '\0';
//     return str - buf;
// }
