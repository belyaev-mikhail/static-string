
template<class T>
using eval = typename T::ret;

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
