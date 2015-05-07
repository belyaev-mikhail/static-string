
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

template<class LTL, class RTL> struct tl_zip;
template<> struct tl_zip<type_list<>, type_list<>> {
    using type = type_list<>;  
};
template<class LH, class RH, class ...LT, class ...RT>
struct tl_zip< type_list<LH, LT...>, type_list<RH, RT...> > {
    static_assert(sizeof...(LT) == sizeof...(RT), "Zipping type lists of different sizes");

    using progress = typename tl_zip< type_list<LT...>, type_list<RT...> >::type;
    using type = tl_cons_t< std::pair<LH, RH>, progress >;
};
template<class LTL, class RTL>
using tl_zip_t = typename tl_zip<LTL, RTL>::type;


template<class LTL, class RTL> struct tl_append;
template<class ...LElems, class ...RElems>
struct tl_append< type_list<LElems...>, type_list<RElems...> > {
    using type = type_list<LElems..., RElems...>;
};
template<class LTL, class RTL>
using tl_append_t = typename tl_append<LTL, RTL>::type;

template<class TL> struct tl_head;
template<class H, class ...T>
struct tl_head<type_list<H, T...>> {
    using type = H;
};
template<class TL> 
using tl_head_t = typename tl_head<TL>::type;

template<class TL> struct tl_tail;
template<class H, class ...T>
struct tl_tail<type_list<H, T...>> {
    using type = type_list<T...>;
};
template<class TL> 
using tl_tail_t = typename tl_tail<TL>::type;

