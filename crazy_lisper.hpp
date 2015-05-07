
#include <cstdint>

template<class T>
using eval = typename T::ret;

template<char Ch, class SS>
struct ss_cons;
template<char Ch, char... Rest>
struct ss_cons<Ch, static_string<Rest...>> {
    using ret = static_string<Ch, Rest...>;
};
template<char Ch, class SS>
using ss_cons_t = eval<ss_cons<Ch, SS>>;



template<char Ch, class SS>
struct ss_contains;
template<char Ch, char... Rest>
struct ss_contains<Ch, static_string<Ch, Rest...>> {
    enum{ value = true };
};
template<char Ch, char Ch2, char... Rest>
struct ss_contains<Ch, static_string<Ch2, Rest...>> {
    enum{ value = ss_contains<Ch, static_string<Rest...>>::value };
};
template<char Ch>
struct ss_contains<Ch, static_string<>> {
    enum{ value = false };
};


template<class SS, char Delim> struct read_while_not;
template<char Delim, char... Chars>
struct read_while_not<static_string<Delim, Chars...>, Delim> {
    using ret = static_string<>;
    using rest = static_string<Chars...>;
};
template<char SomeChar, char Delim, char... Chars>
struct read_while_not<static_string<SomeChar, Chars...>, Delim> {
    using progress = read_while_not<static_string<Chars...>, Delim>;
    using rest = typename progress::rest;
    using ret = ss_cons_t< SomeChar, eval<progress> >;
};
template<class SS, char Delim> 
using read_while_not_t = eval<read_while_not<SS, Delim>>;


using specials = STATIC_STRING("()");

template<char C>
constexpr bool isSpecial() { return ss_contains<C, specials>::value; }

template<class SS, class CurrSym, class Acc, class SFINAE = void>
struct tokenizer;


// tokenize ""         ""      acc = acc
template<class Tokens>
struct tokenizer< static_string<>, static_string<>, Tokens > {
    using ret = Tokens;
};
// tokenize ""         currSym acc = currSym:acc
template<class Tokens, class CurrentSymbol>
struct tokenizer< static_string<>, CurrentSymbol, Tokens > {
    using ret = tl_snoc_t<Tokens, CurrentSymbol>;
};
// tokenize (' ':rest) ""      acc = tokenize rest "" acc
template<class Tokens, char... Rest>
struct tokenizer< static_string<' ', Rest...>, static_string<>, Tokens > {
    using progress = tokenizer< static_string<Rest...>, static_string<>, Tokens >;
    using ret = eval<progress>;
};
// tokenize (' ':rest) currSym acc = tokenize rest "" (currSym:acc)
template<class CurSym, class Tokens, char... Rest>
struct tokenizer< static_string<' ', Rest...>, CurSym, Tokens > {
    using progress = tokenizer<static_string<Rest...>, static_string<>, tl_snoc_t<Tokens, CurSym>>;
    using ret = eval<progress>;
};
// tokenize ('(':rest) ""      acc = tokenize rest "" ("(":acc)
// tokenize (')':rest) ""      acc = tokenize rest "" (")":acc)
template<char Special, class Tokens, char... Rest>
struct tokenizer< static_string<Special, Rest...>, static_string<>, Tokens, std::enable_if_t<isSpecial<Special>()> > {
    using progress = tokenizer<static_string<Rest...>, static_string<>, tl_snoc_t< Tokens, static_string<Special> > >;
    using ret = eval<progress>;
};
// tokenize ('(':rest) currSym acc = tokenize ('(':rest) "" (currSym:acc)
// tokenize (')':rest) currSym acc = tokenize (')':rest) "" (currSym:acc)
template<char Special, class CurSym, class Tokens, char... Rest>
struct tokenizer< static_string<Special, Rest...>, CurSym, Tokens, std::enable_if_t<isSpecial<Special>()> > {
    using progress = tokenizer< static_string<Special, Rest...>, static_string<>, tl_snoc_t<Tokens, CurSym> >;
    using ret = eval<progress>;
};
// tokenize (ih: rest) currSym acc = tokenize rest (currSym ++ [ih]) acc 
template<char First, char... Rest, char... CurSymChars, class Tokens>
struct tokenizer< static_string<First, Rest...>, static_string<CurSymChars...>, Tokens, std::enable_if_t<First != ' ' && !isSpecial<First>() >> {
    using progress = tokenizer<static_string<Rest...>, static_string<CurSymChars..., First>, Tokens>;
    using ret = eval<progress>;
};



template<class SS>
using tokenize = eval<tokenizer<SS, static_string<>, type_list<>>>;


template<class Tokens, class Context, class Stack>
struct lisper;

template<class Token, class... Tokens, class... Context, class Stack>
struct lisper<type_list<Token, Tokens...>, type_list<Context...>, Stack> {
    using newContext = type_list<Context..., Token>;
    using ret = eval< lisper<type_list<Tokens...>, newContext, Stack> >;
};

template<class... Context, class Stack>
struct lisper<type_list<>, type_list<Context...>, Stack> {
    using ret = type_list<Context...>;
};

template<class... Tokens, class... Context, class Stack>
struct lisper<type_list<static_string<'('>, Tokens...>, type_list<Context...>, Stack> {
    using newContext = type_list<>;
    using newStack = tl_cons_t<type_list<Context...>, Stack>;
    using ret = eval< lisper<type_list<Tokens...>, newContext, newStack> >;
};

template<class... Tokens, class... Context, class HStack, class ...TStack>
struct lisper<type_list<static_string<')'>, Tokens...>, type_list<Context...>, type_list<HStack, TStack...>> {
    using newContext = tl_snoc_t<HStack, type_list<Context...>>;
    using newStack = type_list<TStack...>;
    using ret = eval< lisper<type_list<Tokens...>, newContext, newStack> >;
};

template<class Tokens>
using lispFromTokens = eval< lisper<Tokens, type_list<>, type_list<>> >;

template<class SS>
using lispIt = lispFromTokens< tokenize<SS> >;


template<class SS, class SFINAE = void>
struct template_lisp_traits;

#define MAKE_AVAILABLE_TO_LISPER(TYPE) \
template<> struct template_lisp_traits<STATIC_STRING(#TYPE)> \
{ \
    using type = TYPE; \
}; \

MAKE_AVAILABLE_TO_LISPER(bool);
MAKE_AVAILABLE_TO_LISPER(void);

MAKE_AVAILABLE_TO_LISPER(char);
MAKE_AVAILABLE_TO_LISPER(signed char);
MAKE_AVAILABLE_TO_LISPER(unsigned char);
MAKE_AVAILABLE_TO_LISPER(short);
MAKE_AVAILABLE_TO_LISPER(signed short);
MAKE_AVAILABLE_TO_LISPER(unsigned short);
MAKE_AVAILABLE_TO_LISPER(int);
MAKE_AVAILABLE_TO_LISPER(signed int);
MAKE_AVAILABLE_TO_LISPER(unsigned int);
MAKE_AVAILABLE_TO_LISPER(long);
MAKE_AVAILABLE_TO_LISPER(signed long);
MAKE_AVAILABLE_TO_LISPER(unsigned long);
MAKE_AVAILABLE_TO_LISPER(long long);
MAKE_AVAILABLE_TO_LISPER(signed long long);
MAKE_AVAILABLE_TO_LISPER(unsigned long long);
MAKE_AVAILABLE_TO_LISPER(float);
MAKE_AVAILABLE_TO_LISPER(double);
MAKE_AVAILABLE_TO_LISPER(long double);

MAKE_AVAILABLE_TO_LISPER(int8_t);
MAKE_AVAILABLE_TO_LISPER(int16_t);
MAKE_AVAILABLE_TO_LISPER(int32_t);
MAKE_AVAILABLE_TO_LISPER(int64_t);
MAKE_AVAILABLE_TO_LISPER(uint8_t);
MAKE_AVAILABLE_TO_LISPER(uint16_t);
MAKE_AVAILABLE_TO_LISPER(uint32_t);
MAKE_AVAILABLE_TO_LISPER(uint64_t);

MAKE_AVAILABLE_TO_LISPER(size_t);
MAKE_AVAILABLE_TO_LISPER(intptr_t);
MAKE_AVAILABLE_TO_LISPER(uintptr_t);

#define NAMED_TYPE(NAME) typename template_lisp_traits<STATIC_STRING(NAME)>::type

template<class SS>
using resolve_type = typename template_lisp_traits<SS>::type;

template<class EnvMap, class K>
struct env_lookup {
    using val = tl_lookup_t<EnvMap, K>;
    using type = 
        std::conditional<
            std::is_same<val, nulltype>::value, 
            resolve_type<K>, 
            val
        >;
};

template<class EnvMap, class K> using env_lookup_t = typename env_lookup<EnvMap, K>::type;

template<class ...Args>
struct template_lisp_traits<type_list<STATIC_STRING("tuple"), Args...>> {
    using type = std::tuple< resolve_type<Args>... >;
};

template<class ...Args>
struct template_lisp_traits<type_list<STATIC_STRING("pair"), Args...>> {
    static_assert(sizeof...(Args) == 2, "A pair expects two parameters!");
    using type = std::pair< resolve_type<Args>... >;
};

template<class ...Args>
struct template_lisp_traits<type_list<STATIC_STRING("vector"), Args...>> {
    static_assert(sizeof...(Args) == 2, "A pair expects one parameter!");
    using type = std::vector< resolve_type<Args>... >;
};

template<char... s> struct is_number;
template<> struct is_number<>{ enum{ value = true }; };
template<char HChar, char ...TChars> struct is_number<HChar, TChars...>
{
    enum { value = (HChar >= '0' && HChar <= '9') && is_number<TChars...>::value };
};

template<char... s> struct to_number;
template<> struct to_number<>{ enum{ value = 0, score = 0 }; };
template<char HChar> struct to_number<HChar> {
    enum{ value = HChar - '0', score = 10 };
};
template<char HChar, char ...TChars> struct to_number<HChar, TChars...>
{
    using progress = to_number<TChars...>;
    enum { value = (HChar - '0') * progress::score + progress::value, score = progress::score * 10 };
};

template<char ... Digits>
struct template_lisp_traits<static_string<Digits...>, std::enable_if_t<(sizeof...(Digits) > 0) && is_number<Digits...>::value > > {
    enum{ value = to_number<Digits...>::value  };
    using type = std::integral_constant<long, value>;
};

template<>
struct template_lisp_traits<STATIC_STRING("#t")>
{
    using type = std::true_type;
};

template<>
struct template_lisp_traits<STATIC_STRING("#f")>
{
    using type = std::false_type;
};

template<long... s> struct sum;
template<> struct sum<>{ static constexpr long value = 0; };
template<long HVal, long ...TVals> struct sum<HVal, TVals...>
{
    static constexpr long value = HVal + sum<TVals...>::value; 
};


template<class ...Args>
struct template_lisp_traits<type_list<STATIC_STRING("+"), Args...>> {
    using type = std::integral_constant<long, sum<resolve_type<Args>::value...>::value >;
};

template<class Arg>
struct template_lisp_traits<type_list<STATIC_STRING("-"), Arg>> {
    using type = std::integral_constant<long, -resolve_type<Arg>::value >;
};

template<class Lhv, class Rhv>
struct template_lisp_traits<type_list<STATIC_STRING("-"), Lhv, Rhv>> {
    using type = std::integral_constant<long, resolve_type<Lhv>::value - resolve_type<Rhv>::value >;
};

template<class Lhv, class Rhv, class Whatever, class...Rest>
struct template_lisp_traits<type_list<STATIC_STRING("-"), Lhv, Rhv, Whatever, Rest...>> {
    static_assert(sizeof(Lhv) == 0, "Subtraction operator expects one or two arguments");
    using type = void;
};


template<class ...Args> struct is_same_multiple;
template<class First> 
struct  is_same_multiple<First>: std::true_type {};
template<class First> 
struct  is_same_multiple<First, First>: std::true_type {};
template<class First, class Second> 
struct  is_same_multiple<First, Second>: std::false_type {};
template<class First, class ...Rest> 
struct  is_same_multiple<First, First, Rest...> {
    static constexpr bool value = is_same_multiple<First, Rest...>::value;
};
template<class First, class Second, class ...Rest> 
struct  is_same_multiple<First, Second, Rest...> : std::false_type {};

template<class ...Args>
struct template_lisp_traits<type_list<STATIC_STRING("=="), Args...>> {
    using type = std::integral_constant<bool, is_same_multiple<resolve_type<Args>...>::value >;
};

template<class ...Args>
struct template_lisp_traits<type_list<STATIC_STRING("tuple_size"), Args...>> {
    static_assert(sizeof...(Args) == 1, "tuple_size expects 1 parameter");
    using type = std::integral_constant<long, std::tuple_size<resolve_type<Args>...>::value>;
};


template<bool cnd, class Left, class Right>
struct selector;
template<class Left, class Right>
struct selector<true, Left, Right> { using type = resolve_type<Left>; };
template<class Left, class Right>
struct selector<false, Left, Right> { using type = resolve_type<Right>; };

template<class Cond, class Left, class Right, class...Rest>
struct template_lisp_traits<type_list<STATIC_STRING("ite"), Cond, Left, Right, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "ite takes exactly 3 parameters");
    using type = typename selector<resolve_type<Cond>::value, Left, Right>::type;
};

template<>
struct template_lisp_traits<STATIC_STRING("null")> {
    using type = nulltype;
};

template<class Arg, class ...Args>
struct template_lisp_traits<type_list<STATIC_STRING("id"), Arg, Args...>> {
    static_assert(sizeof...(Args) == 0, "tuple_size expects 1 parameter");
    using type = resolve_type<Arg>;
};

struct typetype {};

template<class V> struct is_integral_constant {
    static constexpr bool value = false;
    using type = typetype;
};
template<class V, V v> struct is_integral_constant<std::integral_constant<V, v>> {
    static constexpr bool value = true;
    using type = V;
};

template<class V>
struct template_lisp_traits<type_list<STATIC_STRING("typeof"), V>> {
    using type = typename is_integral_constant<resolve_type<V>>::type;
};

template<>
struct template_lisp_traits<STATIC_STRING("type")> {
    using type = typetype;
};
