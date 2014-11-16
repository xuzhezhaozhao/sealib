
#ifndef __SEAL_FUNCTOR_H__
#define __SEAL_FUNCTOR_H__

#include "typetraits.h"


namespace sea {

// TODO c++14 use auto deduction
template <typename T, typename F, size_t ... Is>
static constexpr auto map_impl(T &&tp, F f, index_sequence<Is...>) ->
decltype(std::make_tuple(f(std::get<Is>(std::forward<T>(tp)))...)) {
	return std::make_tuple(f(std::get<Is>(std::forward<T>(tp)))...);
}

template <typename T, typename F>
static constexpr auto map(T &&tp, F f) ->
decltype(map_impl(std::forward<T>(tp), f, make_index_sequence<std::tuple_size<typename std::decay<T>::type>::value>())) {
	return map_impl(std::forward<T>(tp), f, make_index_sequence<std::tuple_size<typename std::decay<T>::type>::value>());
}


template <typename T, typename F, size_t ... Is>
static constexpr auto apply_impl(T &&tp, F f, index_sequence<Is...>) ->
decltype(f(std::get<Is>(std::forward<T>(tp))...)) {
	return f(std::get<Is>(std::forward<T>(tp))...);
}

template <typename T, typename F>
static constexpr auto apply(T &&tp, F f) ->
decltype(apply_impl(std::forward<T>(tp), f, make_index_sequence<std::tuple_size<typename std::decay<T>::type>::value>())) {
	return apply_impl(std::forward<T>(tp), f, make_index_sequence<std::tuple_size<typename std::decay<T>::type>::value>());
}


template <typename T>
class reduce_constructor {
public:
	template <typename ... Ts>
	T operator()(Ts &&... ps) { return T{std::forward<Ts>(ps)...}; }
};


template <typename M>
class map_inserter {
private:
	M &_m;

	template <typename K, typename V, typename ... Ts>
	void impl(K &&k, V &&v, Ts ... ps) {
		_m.insert({std::forward<K>(k), std::forward<V>(v)});
		impl(std::forward<Ts>(ps)...);
	}
	void impl() {}
public:
	map_inserter(M &m): _m(m) {}

	template <typename ... Ts>
	void operator()(Ts &&... ps) { impl(std::forward<Ts>(ps)...); }
};

template <typename O>
class repeat_operator {
private:
	O _o;

	template <typename T, typename ... Ts>
	T impl(T a, T b, Ts ... ps) { return impl(_o(a, b), ps...); }

	template <typename T>
	T impl(T a) { return a; }

public:
	repeat_operator() = default;
	repeat_operator(O o): _o(o) {}

	template <typename ... Ts>
	auto operator()(Ts ... ps) -> decltype(this->impl(ps...)) { return impl(ps...); }
};

/**
 * 下面两个functor的功能类似: 取得pair的第一个或第二个参数
 */
template <typename> struct get_first;
template <typename T, typename U>
struct get_first<std::pair<T, U>> {
	typedef T type;
	type &operator()(std::pair<T, U> &p) const { return p.first; }
	const type &operator()(const std::pair<T, U> &p) const { return p.first; }
};

template <typename> struct get_second;
template <typename T, typename U>
struct get_second<std::pair<T, U>> {
	typedef U type;
	type &operator()(std::pair<T, U> &p) const { return p.second; }
	const type &operator()(const std::pair<T, U> &p) const { return p.second; }
};


}

#endif // __SEAL_FUNCTOR_H__

