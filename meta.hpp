
#include <type_traits>

enum dummy{ enable };

template<class Bool>
using EnableIfQ = typename std::enable_if<Bool::value, dummy>::type;

template<class ...Elems> struct type_list{};

template<class H, class T> struct tl_cons;
template<class H, class ...Elems> 
struct tl_cons<H, type_list<Elems...>>
{
    using type = type_list<H, Elems...>;
};
template<class H, class TL> using tl_cons_t = typename tl_cons<H, TL>::type;

template<class T, class H> struct tl_snoc;
template<class H, class ...Elems> 
struct tl_snoc<type_list<Elems...>, H>
{
    using type = type_list<Elems..., H>;
};
template<class TL, class H> using tl_snoc_t = typename tl_snoc<TL, H>::type;

template<class TL> struct tl_reverse;
template<class H, class ...T>
struct tl_reverse<type_list<H, T...>> {
    using newTail = typename tl_reverse<type_list<T...>>::type;
    using type = tl_snoc_t<newTail, H>;
};
template<>
struct tl_reverse<type_list<>> {
    using type = type_list<>;
};
template<class TL> using tl_reverse_t = typename tl_reverse<TL>::type;


struct nulltype{};

template<class TL, class Key> struct tl_lookup;
template<class Key>
struct tl_lookup< type_list<>, Key > {
    using type = nulltype;
};
template<class HK, class HV, class ...T>
struct tl_lookup< type_list<std::pair<HK, HV>, T...>, HK > {
    using type = HV;
};
template<class Key, class HK, class HV, class ...T>
struct tl_lookup< type_list<std::pair<HK, HV>, T...>, Key > {
    using type = typename tl_lookup<type_list<T...>, Key>::type;
};
template<class TL, class Key> using tl_lookup_t = typename tl_lookup<TL, Key>::type;
