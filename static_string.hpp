#include <iostream>

template<char ...Args>
struct static_string {
    enum { length = sizeof...(Args) };
    typedef const char(&array_ref)[sizeof...(Args)+1];

    static array_ref c_str() {
        static const char output[]{ Args..., '\0' };
        return output;
    }

    constexpr size_t size() { 
        return length; 
    }

    operator array_ref() const {
        return c_str();
    }

    operator const char*() const {
        return c_str();
    }

    friend std::ostream& operator<<(std::ostream& ost, static_string<Args...>) {
        return ost << static_string::c_str();
    }
};

namespace impl_ {

template<size_t Sz>
constexpr char ss_at(char const(&s)[Sz], size_t i) {
    return i >= Sz ? '\0' : s[i];
}

template<char C, class SS>
struct ss_push_front;

template<char C, char ...Args>
struct ss_push_front < C, static_string <Args...> > {
    using type = static_string<C, Args...>;
};

template<char C, class SS>
using ss_push_front_t = typename ss_push_front<C, SS>::type;

template<class SS>
struct ss_truncate;

template<char... contents>
struct ss_truncate< static_string<'\0', contents...> > {
    using type = static_string<>;
};

template<char h, char... contents>
struct ss_truncate< static_string<h, contents...> > {
    using tail = static_string<contents...>;
    using trunc = typename ss_truncate< tail >::type;
    using type = ss_push_front_t<h, trunc>;
};

template<class SS>
using ss_truncate_t = typename ss_truncate<SS>::type;

template<char ...Args>
using ss_make_string = ss_truncate_t< static_string< Args... > >;

} /* namespace impl_ */

#define STATIC_STRING(...) impl_::ss_make_string< \
    impl_::ss_at((__VA_ARGS__),  0U), \
    impl_::ss_at((__VA_ARGS__),  1U), \
    impl_::ss_at((__VA_ARGS__),  2U), \
    impl_::ss_at((__VA_ARGS__),  3U), \
    impl_::ss_at((__VA_ARGS__),  4U), \
    impl_::ss_at((__VA_ARGS__),  5U), \
    impl_::ss_at((__VA_ARGS__),  6U), \
    impl_::ss_at((__VA_ARGS__),  7U), \
    impl_::ss_at((__VA_ARGS__),  8U), \
    impl_::ss_at((__VA_ARGS__),  9U), \
    impl_::ss_at((__VA_ARGS__), 10U), \
    impl_::ss_at((__VA_ARGS__), 11U), \
    impl_::ss_at((__VA_ARGS__), 12U), \
    impl_::ss_at((__VA_ARGS__), 13U), \
    impl_::ss_at((__VA_ARGS__), 14U), \
    impl_::ss_at((__VA_ARGS__), 15U), \
    impl_::ss_at((__VA_ARGS__), 16U), \
    impl_::ss_at((__VA_ARGS__), 17U), \
    impl_::ss_at((__VA_ARGS__), 18U), \
    impl_::ss_at((__VA_ARGS__), 19U), \
    impl_::ss_at((__VA_ARGS__), 20U), \
    impl_::ss_at((__VA_ARGS__), 21U), \
    impl_::ss_at((__VA_ARGS__), 22U), \
    impl_::ss_at((__VA_ARGS__), 23U), \
    impl_::ss_at((__VA_ARGS__), 24U), \
    impl_::ss_at((__VA_ARGS__), 25U), \
    impl_::ss_at((__VA_ARGS__), 26U), \
    impl_::ss_at((__VA_ARGS__), 27U), \
    impl_::ss_at((__VA_ARGS__), 28U), \
    impl_::ss_at((__VA_ARGS__), 29U), \
    impl_::ss_at((__VA_ARGS__), 20U), \
    impl_::ss_at((__VA_ARGS__), 21U), \
    impl_::ss_at((__VA_ARGS__), 22U), \
    impl_::ss_at((__VA_ARGS__), 23U), \
    impl_::ss_at((__VA_ARGS__), 24U), \
    impl_::ss_at((__VA_ARGS__), 25U), \
    impl_::ss_at((__VA_ARGS__), 26U), \
    impl_::ss_at((__VA_ARGS__), 27U), \
    impl_::ss_at((__VA_ARGS__), 28U), \
    impl_::ss_at((__VA_ARGS__), 29U), \
    impl_::ss_at((__VA_ARGS__), 30U), \
    impl_::ss_at((__VA_ARGS__), 31U), \
    impl_::ss_at((__VA_ARGS__), 32U), \
    impl_::ss_at((__VA_ARGS__), 33U), \
    impl_::ss_at((__VA_ARGS__), 34U), \
    impl_::ss_at((__VA_ARGS__), 35U), \
    impl_::ss_at((__VA_ARGS__), 36U), \
    impl_::ss_at((__VA_ARGS__), 37U), \
    impl_::ss_at((__VA_ARGS__), 38U), \
    impl_::ss_at((__VA_ARGS__), 39U), \
    impl_::ss_at((__VA_ARGS__), 30U), \
    impl_::ss_at((__VA_ARGS__), 31U), \
    impl_::ss_at((__VA_ARGS__), 32U), \
    impl_::ss_at((__VA_ARGS__), 33U), \
    impl_::ss_at((__VA_ARGS__), 34U), \
    impl_::ss_at((__VA_ARGS__), 35U), \
    impl_::ss_at((__VA_ARGS__), 36U), \
    impl_::ss_at((__VA_ARGS__), 37U), \
    impl_::ss_at((__VA_ARGS__), 38U), \
    impl_::ss_at((__VA_ARGS__), 39U)  \
>
