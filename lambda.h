
#include <cstddef>
#include <type_traits>
#include <tuple>

template <size_t i> struct arg {};

typedef arg<1> _1;
typedef arg<2> _2;
typedef arg<3> _3;
typedef arg<4> _4;
typedef arg<5> _5;


#define def_asmd(n, o)	\
template <typename t, typename s, typename ... ts>	\
struct n : public n<n<t, s>, ts...>::type {};	\
template <typename t, typename s>	\
struct n<t, s> : public std::integral_constant<decltype(t::value o s::value), t::value o s::value> {};

def_asmd(add_, +)
def_asmd(sub_, -)
def_asmd(mul_, *)
def_asmd(div_, /)

#undef def_asmd


template <template <typename...> class f> struct make_mf {
	template <typename ... ss>
	struct apply {
		typedef typename f<ss...>::type type;
	};
};

template <typename le> struct lambda {
	template <typename ... ss>
	struct apply {
		typedef le type;
	};
};

template <template <typename ...> class f> struct lambda<make_mf<f>> : public make_mf<f> {};

template <typename t, typename = void> struct get_type {
	typedef t type;
};

template <typename...> struct always_void {
	typedef void type;
};

template <typename t> struct get_type<t, typename always_void<typename t::type>::type> {
	typedef typename t::type type;
};

template <size_t i> struct lambda<arg<i>> {
	template <typename ... ss>
	struct apply {
		typedef typename std::tuple_element<i-1, std::tuple<ss...>>::type type;
	};
};

template <template <typename ...> class le, typename ... ts>
struct lambda<le<ts...>> {
	template <typename ... ss>
	struct apply {
		typedef le<typename lambda<ts>::template apply<ss...>::type...> impl;
		typedef typename get_type<impl>::type type;
	};
};

template <typename f, typename ... ts> struct apply {
	typedef typename lambda<f>::template apply<ts...>::type type;
};


template <int v> struct int_ : public std::integral_constant<int, v> {};

static_assert(
		apply<
			div_<
				mul_<
					add_<_1, _1, int_<2>>,
					add_<_2, _2, int_<4>>
				>,
				int_<2>,
				sub_<
					int_<5>,
					int_<1>
				>
			>,
			int_<3>,
			int_<2>
		>::type::value == 8, "lambda");

template <typename _1, typename _2> using xxx = mul_<add_<_1, _2>, add_<_1, _2>>;
static_assert(xxx<int_<3>, int_<5>>::value == 64, "lambda");

#include <map>
typedef std::map<int, _1> im;
static_assert(std::is_same<apply<im, double>::type, std::map<int, double>>::value, "lambda");

static_assert(std::is_same<apply<make_mf<std::add_pointer>, int>::type, int *>::value, "lambda");
static_assert(std::is_same<apply<std::add_pointer<_1>, int>::type, int *>::value, "lambda");

