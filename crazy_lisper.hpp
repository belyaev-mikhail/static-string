
#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <list>
#include <deque>
#include <type_traits>

#include "crazy_lisp_parser.hpp"

// lookup errors are represented by a class
template<class...> struct error_type {};
// these are just markers
struct env_begin{};
struct env_end{};

// literal "type"
struct typetype {};
// literal "function"
struct functiontype {};
// literal "symbol"
struct symboltype {};

// this trait represents one evaluation step
// general case of a trait - resolves to an error
template<class Env, class SS, class SFINAE = void>
struct template_lisp_traits {
    using type = error_type<env_begin, Env, env_end, SS>;
};

/* the actuall name resolution is an env lookup + builtin search */
template<class Env, class SS>
struct type_resolver {
    using type = typename template_lisp_traits<Env, SS>::type;
};
// null is resolved to null
template<class Env>
struct type_resolver<Env, nulltype> {
    using type = nulltype;
};
// type is resolved to type
template<class Env>
struct type_resolver<Env, typetype> {
    using type = typetype;
};
// numeric constants are resolved to themselves
template<class Env, class V, V v>
struct type_resolver<Env, std::integral_constant<V, v>> {
    using type = std::integral_constant<V, v>;
};
// names are resolved to env lookup 
// and if that fails, tries the general case
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
// shortcut template for type_resolver
template<class Env, class SS>
using resolve_type = typename type_resolver<Env, SS>::type;

// literal "null" (nulltype is defined in meta.hpp)
template<class Env>
struct template_lisp_traits<Env, STATIC_STRING("null")> {
    using type = nulltype;
};

// literal "type" (typetype is defined before)
template<class Env>
struct template_lisp_traits<Env, STATIC_STRING("type")> {
    using type = typetype;
};

// literal "function" (functiontype is defined before)
template<class Env>
struct template_lisp_traits<Env, STATIC_STRING("function")> {
    using type = functiontype;
};

// literal "symbol" (symboltype is defined before)
template<class Env>
struct template_lisp_traits<Env, STATIC_STRING("symbol")> {
    using type = symboltype;
};

// lambdas 
template<class Args, class Body, class Env> struct lambda;
template<class ...Args, class Body, class Env>
struct lambda<type_list<Args...>, Body, Env> {

    // assign all de facto arguments to declared arguments and resolve the body in the context

    template<class ...RArgs>
    using apply = resolve_type< tl_append_t< tl_zip_t<type_list<Args...>, type_list<RArgs...>>, Env >, Body>;
};

template<class Env, class Args, class Body, class InnerEnv>
struct template_lisp_traits<Env, lambda<Args, Body, InnerEnv>> {
    using type = lambda<Args, Body, InnerEnv>;
};

// quotation is a type wrapper
template<class T> struct quoted{};
template<class Q> struct unquote { using type = Q; };
template<class Q> struct unquote<quoted<Q>> { using type = Q; };
template<class Q>
using unquote_t = typename unquote<Q>::type;

// all the standard C++ types are available to lisper
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

#undef MAKE_AVAILABLE_TO_LISPER

#define MAKE_TEMPLATE_AVAILABLE_TO_LISPER(NAME, EXPECTED_ARGS) \
    template<class Env, class ...Args> \
    struct template_lisp_traits<Env, type_list<STATIC_STRING(#NAME), Args...>> { \
        static_assert(sizeof...(Args) == EXPECTED_ARGS, #NAME " : wrong number of arguments"); \
        using type = std::NAME< resolve_type<Env, Args>... >; \
    };\

MAKE_TEMPLATE_AVAILABLE_TO_LISPER(tuple, sizeof...(Args));
MAKE_TEMPLATE_AVAILABLE_TO_LISPER(pair, 2);
MAKE_TEMPLATE_AVAILABLE_TO_LISPER(vector, 1);
MAKE_TEMPLATE_AVAILABLE_TO_LISPER(list, 1);
MAKE_TEMPLATE_AVAILABLE_TO_LISPER(deque, 1);
MAKE_TEMPLATE_AVAILABLE_TO_LISPER(map, 2);
MAKE_TEMPLATE_AVAILABLE_TO_LISPER(unordered_map, 2);
MAKE_TEMPLATE_AVAILABLE_TO_LISPER(set, 1);
MAKE_TEMPLATE_AVAILABLE_TO_LISPER(unordered_set, 1);

#undef MAKE_TEMPLATE_AVAILABLE_TO_LISPER

#define MAKE_RENAMED_TEMPLATE_AVAILABLE_TO_LISPER(NAME, STLNAME) \
    template<class Env, class Arg, class ...Args> \
    struct template_lisp_traits<Env, type_list<STATIC_STRING(#NAME), Arg, Args...>> { \
        static_assert(sizeof...(Args) == 0, #NAME " : wrong number of arguments"); \
        using type = std::STLNAME< resolve_type<Env, Arg> >; \
    };\

MAKE_RENAMED_TEMPLATE_AVAILABLE_TO_LISPER(const, add_const_t);
MAKE_RENAMED_TEMPLATE_AVAILABLE_TO_LISPER(volatile, add_volatile_t);
MAKE_RENAMED_TEMPLATE_AVAILABLE_TO_LISPER(unsigned, make_unsigned_t);
MAKE_RENAMED_TEMPLATE_AVAILABLE_TO_LISPER(signed, make_signed_t);

#undef MAKE_RENAMED_TEMPLATE_AVAILABLE_TO_LISPER

#define MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(NAME, STLNAME) \
    template<class Env, class Arg, class ...Args> \
    struct template_lisp_traits<Env, type_list<STATIC_STRING(#NAME), Arg, Args...>> { \
        static_assert(sizeof...(Args) == 0, #NAME " : wrong number of arguments"); \
        using type = std::integral_constant<bool,  std::STLNAME< resolve_type<Env, Arg> >::value >; \
    };\

MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(void?, is_void);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(null_pointer?, is_null_pointer);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(integral?, is_integral);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(floating_point?, is_floating_point);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(array?, is_array);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(enum?, is_enum);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(union?, is_union);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(class?, is_class);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(function?, is_function);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(pointer?, is_pointer);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(lvalue_reference?, is_lvalue_reference);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(rvalue_reference?, is_rvalue_reference);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(member_object_pointer?, is_member_object_pointer);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(member_function_pointer?, is_member_function_pointer);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(fundamental?, is_fundamental);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(arithmetic?, is_arithmetic);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(scalar?, is_scalar);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(object?, is_object);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(compound?, is_compound);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(reference?, is_reference);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(member_pointer?, is_member_pointer);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(const?, is_const);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(volatile?, is_volatile);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(trivial?, is_trivial);
// MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(trivially_copyable?, is_trivially_copyable);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(standard_layout?, is_standard_layout);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(pod?, is_pod);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(literal_type?, is_literal_type);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(empty?, is_empty);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(polymorphic?, is_polymorphic);
// MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(final?, is_final);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(abstract?, is_abstract);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(signed?, is_signed);
MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER(unsigned?, is_unsigned);

#undef MAKE_RENAMED_CHECKER_AVAILABLE_TO_LISPER

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
struct template_lisp_traits<Env, STATIC_STRING("true")>
{
    using type = std::true_type;
};

template<class Env>
struct template_lisp_traits<Env, STATIC_STRING("false")>
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

template<class Env, class Lhv, class Rhv>
struct template_lisp_traits<Env, type_list<STATIC_STRING("/"), Lhv, Rhv>> {
    using type = std::integral_constant<long, resolve_type<Env, Lhv>::value / resolve_type<Env, Rhv>::value >;
};
template<class Env, class Lhv, class Rhv, class Whatever, class...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("/"), Lhv, Rhv, Whatever, Rest...>> {
    static_assert(sizeof(Lhv) == 0, "Division operator expects exactly two arguments");
    using type = error_type<Env, STATIC_STRING("/"), Lhv, Rhv, Whatever, Rest...>;
};

template<class Env, class Lhv, class Rhv>
struct template_lisp_traits<Env, type_list<STATIC_STRING("%"), Lhv, Rhv>> {
    using type = std::integral_constant<long, resolve_type<Env, Lhv>::value % resolve_type<Env, Rhv>::value >;
};
template<class Env, class Lhv, class Rhv, class Whatever, class...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("%"), Lhv, Rhv, Whatever, Rest...>> {
    static_assert(sizeof(Lhv) == 0, "Remainder operator expects exactly two arguments");
    using type = error_type<Env, STATIC_STRING("%"), Lhv, Rhv, Whatever, Rest...>;
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
struct template_lisp_traits<Env, type_list<STATIC_STRING("="), Args...>> {
    using type = std::integral_constant<bool, is_same_multiple<resolve_type<Env, Args>...>::value >;
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

template<class Env, class Cond, class Left, class Right, class...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("if"), Cond, Left, Right, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "if takes exactly 3 parameters");
    using type = typename selector<Env, resolve_type<Env, Cond>::value, Left, Right>::type;
};

template<class Env, class Cond, class Left, class Right, class...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("?"), Cond, Left, Right, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "? takes exactly 3 parameters");
    using type = typename selector<Env, resolve_type<Env, Cond>::value, Left, Right>::type;
};

template<class Env, class Arg, class ...Args>
struct template_lisp_traits<Env, type_list<STATIC_STRING("id"), Arg, Args...>> {
    static_assert(sizeof...(Args) == 0, "id expects 1 parameter");
    using type = resolve_type<Env, Arg>;
};


template<class V> struct type_checker {
    using type = typetype;
};
template<class V, V v> struct type_checker<std::integral_constant<V, v>> {
    using type = V;
};
template<class Args, class Body, class Env> struct type_checker< lambda<Args, Body, Env> > {
    using type = functiontype;
};
template<class Arg> struct type_checker< quoted<Arg> > {
    using type = symboltype;
};
template<class Env, class V>
struct template_lisp_traits<Env, type_list<STATIC_STRING("typeof"), V>> {
    using type = typename type_checker<resolve_type<Env, V>>::type;
};


// quoted stuff

template<class Env, char... SymName>
struct template_lisp_traits<Env, static_string<'$', SymName...>> {
    using type = quoted<static_string<SymName...>>;
};
template<class Env>
struct template_lisp_traits<Env, STATIC_STRING("nil")>
{
    using type = quoted<type_list<>>;
};
template<class Env, class Arg, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("quote"), Arg, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal quote: too many arguments");
    using type = quoted<Arg>;
};
template<class Env, class ...Args>
struct template_lisp_traits<Env, type_list<STATIC_STRING("lst"), Args...>> {
    using type = quoted<type_list<unquote_t<resolve_type<Env, Args>>...>>;
};
template<class Env, class Arg, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("car"), Arg, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal car: too many arguments");
    using type = tl_head_t<unquote_t<resolve_type<Env, Arg>>>;
};
template<class Env, class Arg, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("cdr"), Arg, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal cdr: too many arguments");
    using type = quoted<tl_tail_t<unquote_t<resolve_type<Env, Arg>>>>;
};
template<class Env, class HArg, class TArg, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("cons"), HArg, TArg, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal cons: too many arguments");
    using type = quoted<tl_cons_t<unquote_t<resolve_type<Env, HArg>>, unquote_t<resolve_type<Env, TArg>>>>;
};

template<class Env, class Body, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("eval"), Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal eval: too many arguments");
    using newEnv = type_list<>;
    using type = resolve_type<newEnv, unquote_t<resolve_type<Env, unquote_t<Body>>>>;
};

// quoted stuff is guared against anything except eval
template<class Env, class T>
struct template_lisp_traits<Env, quoted<T>> {
    using type = quoted<T>;
};


// let form : one binding
// ex: (let (a 2) (+ a 5)) -> 7
template<class Env, char... Name, class Value, class Body, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("let"), type_list<static_string<Name...>, Value>, Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal let: too many arguments");
    using newEnv = tl_cons_t< std::pair<static_string<Name...>, resolve_type<Env, Value>>, Env >;
    using type = resolve_type<newEnv, Body>;
};

// let form : many bindings
// ex: (let ((a 2)(b 5)) (+ a b 8)) -> 15
template<class Env, char... Name, class Value, class ...TBindings, class Body, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("let"), type_list<type_list<static_string<Name...>, Value>, TBindings...>, Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal let: too many arguments");
    using newEnv = tl_cons_t< std::pair<static_string<Name...>, resolve_type<Env, Value>>, Env >;
    using progress = template_lisp_traits<newEnv, type_list<STATIC_STRING("let"), type_list<TBindings...>, Body, Rest...>>;
    using type = typename progress::type;
};
// let form : empty binding
// ex: (let () 15) -> 15
template<class Env, class Body, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("let"), type_list<>, Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal let: too many arguments");
    using type = resolve_type<Env, Body>;
};

// lambda form
template<class Env, class Body, class ...Args, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("lambda"), type_list<Args...>, Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal lambda: too many arguments");

    using type = lambda<type_list<Args...>, Body, Env>;
};
template<class Env, class Body, class ...Args, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("\\"), type_list<Args...>, Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal lambda: too many arguments");

    using type = lambda<type_list<Args...>, Body, Env>;
};

// recursive let: adds the binding to env while resolving the binding itself
// only single-bind form allowed
template<class Env, char... Name, class Value, class Body, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("letrec"), type_list<static_string<Name...>, Value>, Body, Rest...>> {
    static_assert(sizeof...(Rest) == 0, "illegal letrec: too many arguments");
    using recEnv = tl_cons_t< std::pair<static_string<Name...>, Value>, Env >;
    using newEnv = tl_cons_t< std::pair<static_string<Name...>, resolve_type<recEnv, Value>>, recEnv >;
    using type = resolve_type<newEnv, Body>;
};

// apply spec form
template<class Env, class Fun, class ...Rest>
struct template_lisp_traits<Env, type_list<STATIC_STRING("apply"), Fun, Rest...>> {
    using lam = resolve_type<Env, Fun>;
    using type = typename lam::template apply<resolve_type<Env, Rest>...>;
};

template<class Env, class Unknown, class ...Rest>
struct template_lisp_traits<Env, type_list<Unknown, Rest...>> {
    using lam = resolve_type<Env, Unknown>;
    using type = typename lam::template apply<resolve_type<Env, Rest>...>;
};



