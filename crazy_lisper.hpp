
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


template<class Env, class SS, class SFINAE = void>
struct template_lisp_traits {
    using type = nulltype;
};

#define MAKE_AVAILABLE_TO_LISPER(TYPE) \
template<class Env> struct template_lisp_traits<Env, STATIC_STRING(#TYPE)> \
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

#define NAMED_TYPE(NAME) typename template_lisp_traits<type_list<>, STATIC_STRING(NAME)>::type

template<class Env, class SS>
struct type_resolver {
    using type = typename template_lisp_traits<Env, SS>::type;
};

template<class Env>
struct type_resolver<Env, nulltype> {
    using type = nulltype;
};

template<class Env, class V, V v>
struct type_resolver<Env, std::integral_constant<V, v>> {
    using type = std::integral_constant<V, v>;
};

template<class Env, char ...Name>
struct type_resolver<Env, static_string<Name...>> {
    using SS = static_string<Name...>;
    using lookup = tl_lookup_t<Env, SS>;
    using type = std::conditional_t< 
        std::is_same<lookup, nulltype>::value, 
        typename template_lisp_traits<Env, SS>::type,
        typename type_resolver<Env, lookup>::type
     >;
};

template<class Env, class SS>
using resolve_type = typename type_resolver<Env, SS>::type;

template<class Env, class ...Args>
struct template_lisp_traits<Env, type_list<STATIC_STRING("tuple"), Args...>> {
    using type = std::tuple< resolve_type<Env, Args>... >;
};

template<class Env, class ...Args>
struct template_lisp_traits<Env, type_list<STATIC_STRING("pair"), Args...>> {
    static_assert(sizeof...(Args) == 2, "A pair expects two parameters!");
    using type = std::pair< resolve_type<Env, Args>... >;
};

template<class Env, class ...Args>
struct template_lisp_traits<Env, type_list<STATIC_STRING("vector"), Args...>> {
    static_assert(sizeof...(Args) == 2, "A pair expects one parameter!");
    using type = std::vector< resolve_type<Env, Args>... >;
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

template<class Env, char ... Digits>
struct template_lisp_traits<Env, static_string<Digits...>, std::enable_if_t<(sizeof...(Digits) > 0) && is_number<Digits...>::value > > {
    enum{ value = to_number<Digits...>::value  };
    using type = std::integral_constant<long, value>;
};

template<class Env>
struct template_lisp_traits<Env, STATIC_STRING("#t")>
{
    using type = std::true_type;
};

template<class Env>
struct template_lisp_traits<Env, STATIC_STRING("#f")>
{
    using type = std::false_type;
};

template<long... s> struct sum;
template<> struct sum<>{ static constexpr long value = 0; };
template<long HVal, long ...TVals> struct sum<HVal, TVals...>
{
    static constexpr long value = HVal + sum<TVals...>::value; 
};


template<class Env, class ...Args>
struct template_lisp_traits<Env, type_list<STATIC_STRING("+"), Args...>> {
    using type = std::integral_constant<long, sum<resolve_type<Env, Args>::value...>::value >;
};

template<long... s> struct product;
template<> struct product<>{ static constexpr long value = 1; };
template<long HVal, long ...TVals> struct product<HVal, TVals...>
{
    static constexpr long value = HVal * product<TVals...>::value; 
};


template<class Env, class ...Args>
struct template_lisp_traits<Env, type_list<STATIC_STRING("*"), Args...>> {
    using type = std::integral_constant<long, product<resolve_type<Env, Args>::value...>::value >;
};

template<class Env, class Arg>
struct template_lisp_traits<Env, type_list<STATIC_STRING("-"), Arg>> {
    using type = std::integral_constant<long, -resolve_type<Env, Arg>::value >;
};

template<class Env, class Lhv, class Rhv>
struct template_lisp_traits<Env, type_list<STATIC_STRING("-"), Lhv, Rhv>> {
    using type = std::integral_constant<long, resolve_type<Env, Lhv>::value - resolve_type<Env, Rhv>::value >;
};

template<class Env, class Lhv, class Rhv, class Whatever, class...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("-"), Lhv, Rhv, Whatever, Rest...>> {
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

template<class Env, class ...Args>
struct template_lisp_traits<Env, type_list<STATIC_STRING("=="), Args...>> {
    using type = std::integral_constant<bool, is_same_multiple<resolve_type<Env, Args>...>::value >;
};

template<class Env, class ...Args>
struct template_lisp_traits<Env, type_list<STATIC_STRING("tuple_size"), Args...>> {
    static_assert(sizeof...(Args) == 1, "tuple_size expects 1 parameter");
    using type = std::integral_constant<long, std::tuple_size<resolve_type<Env, Args>...>::value>;
};


template<class Env, bool cnd, class Left, class Right>
struct selector;
template<class Env, class Left, class Right>
struct selector<Env, true, Left, Right> { using type = resolve_type<Env, Left>; };
template<class Env, class Left, class Right>
struct selector<Env, false, Left, Right> { using type = resolve_type<Env, Right>; };

template<class Env, class Cond, class Left, class Right, class...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("ite"), Cond, Left, Right, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "ite takes exactly 3 parameters");
    using type = typename selector<Env, resolve_type<Env, Cond>::value, Left, Right>::type;
};

template<class Env>
struct template_lisp_traits<Env, STATIC_STRING("null")> {
    using type = nulltype;
};

template<class Env, class Arg, class ...Args>
struct template_lisp_traits<Env, type_list<STATIC_STRING("id"), Arg, Args...>> {
    static_assert(sizeof...(Args) == 0, "tuple_size expects 1 parameter");
    using type = resolve_type<Env, Arg>;
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

template<class Env, class V>
struct template_lisp_traits<Env, type_list<STATIC_STRING("typeof"), V>> {
    using type = typename is_integral_constant<resolve_type<Env, V>>::type;
};

template<class Env>
struct template_lisp_traits<Env, STATIC_STRING("type")> {
    using type = typetype;
};

template<class Env, char... SymName>
struct template_lisp_traits<Env, static_string<'$', SymName...>> {
    using type = static_string<SymName...>;
};


template<class Env, class Arg, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("quote"), Arg, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal quote: too many arguments");
    using type = Arg;
};

template<class Env, class Body, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("eval"), Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal eval: too many arguments");
    using newEnv = type_list<>;
    using type = resolve_type<newEnv, resolve_type<Env, Body>>;
};

template<class Env, class ...Args>
struct template_lisp_traits<Env, type_list<STATIC_STRING("list"), Args...>> {
    using type = type_list<resolve_type<Env, Args>...>;
};

template<class Env, char... Name, class Value, class Body, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("let"), type_list<static_string<Name...>, Value>, Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal let: too many arguments");
    using newEnv = tl_cons_t< std::pair<static_string<Name...>, resolve_type<Env, Value>>, Env >;
    using type = resolve_type<newEnv, Body>;
};


template<class Env, char... Name, class Value, class ...TBindings, class Body, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("let"), type_list<type_list<static_string<Name...>, Value>, TBindings...>, Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal let: too many arguments");
    using newEnv = tl_cons_t< std::pair<static_string<Name...>, resolve_type<Env, Value>>, Env >;
    using progress = template_lisp_traits<newEnv, type_list<STATIC_STRING("let"), type_list<TBindings...>, Body, Rest...>>;
    using type = typename progress::type;
};

template<class Env, class Body, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("let"), type_list<>, Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal let: too many arguments");
    using type = resolve_type<Env, Body>;
};


template<class Args, class Body, class Env>
struct lambda;
template<class ...Args, class Body, class Env>
struct lambda<type_list<Args...>, Body, Env> {
    template<class ...RArgs>
    using apply = resolve_type< tl_append_t< tl_zip_t<type_list<Args...>, type_list<RArgs...>>, Env >, Body>;
};

template<class Env, class Args, class Body, class InnerEnv>
struct template_lisp_traits<Env, lambda<Args, Body, InnerEnv>> {
    using type = lambda<Args, Body, InnerEnv>;
};

template<class Env, class Body, class ...Args, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("lambda"), type_list<Args...>, Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal lambda: too many arguments");

    using type = lambda<type_list<Args...>, Body, Env>;
};

template<class Env, char... Name, class Value, class Body, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("letrec"), type_list<static_string<Name...>, Value>, Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal letrec: too many arguments");
    using recEnv = tl_cons_t< std::pair<static_string<Name...>, Value>, Env >;
    using newEnv = tl_cons_t< std::pair<static_string<Name...>, resolve_type<recEnv, Value>>, recEnv >;
    using type = resolve_type<newEnv, Body>;
};


template<class Env, class Fun, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("apply"), Fun, Rest...>> {
    using lam = resolve_type<Env, Fun>;
    using type = typename lam::template apply<resolve_type<Env, Rest>...>;
};


