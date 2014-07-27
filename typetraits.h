
#ifndef __SEAL_TRAITS_H__
#define __SEAL_TRAITS_H__

#include <array>
#include <tuple>
#include <type_traits>
#include <utility>

#include <cstddef>
#include <cstdint>


namespace sea {

template <typename  T> struct incomplete_t;
template <intmax_t  V> struct incomplete_i;
template <uintmax_t V> struct incomplete_u;

struct empty {};

template <bool V = false> struct bool_constant : public std::false_type {
	typedef bool_constant type;
};
template <> struct bool_constant<true> : public std::true_type {
	typedef int enable;
	typedef bool_constant type;
};
typedef bool_constant<false> false_type;
typedef bool_constant<true> true_type;


template <typename, typename T>
struct get_2nd_type {
	typedef T type;
};

template <typename T>
using make_void = get_2nd_type<T, void>;


template <bool V>
using always_void = make_void<void>;


template <typename T, typename F>
static constexpr T
type_cast_impl(F &&f, ...) {
	return static_cast<T>(f);
}

template <typename T, typename F>
static constexpr T
type_cast_impl(F &&f, typename std::add_pointer<decltype(std::declval<F>().template as<T>())>::type) {
	return std::forward<F>(f).template as<T>();
}

template <typename T, typename F>
static constexpr T
type_cast(F &&f) {
	return type_cast_impl<T>(std::forward<F>(f), nullptr);
}



template <typename T, T ... Is> struct integer_sequence {
	typedef T value_type;
	typedef integer_sequence type;
	static constexpr size_t size() { return sizeof...(Is); }
};

template <typename T, T N, T ... Is> struct make_iseq_impl {
	typedef typename std::conditional<N == 0, integer_sequence<T, Is...>, make_iseq_impl<T, N-1, N-1, Is...>>::type::type type;
};

template <typename T, T N>
using make_integer_sequence = typename make_iseq_impl<T, N>::type;

template <size_t... Is>
using index_sequence = integer_sequence<size_t, Is...>;

template <size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;


template <typename T, typename U>
struct replace_core {
	typedef U type;
};
template <typename T, typename U>
struct replace_core<T &, U> {
	typedef typename replace_core<T, U>::type &type;
};
template <typename T, typename U>
struct replace_core<T &&, U> {
	typedef typename replace_core<T, U>::type &&type;
};
template <typename T, typename U>
struct replace_core<T *, U> {
	typedef typename replace_core<T, U>::type *type;
};
template <typename T, size_t N, typename U>
struct replace_core<T [N], U> {
	typedef typename replace_core<T, U>::type type[N];
};
template <typename T, typename U>
struct replace_core<const T, U> {
	typedef const typename replace_core<T, U>::type type;
};


template <template <typename...> class, typename, typename> struct repeat_impl;

template <template <typename...> class C, typename T, size_t N, size_t ... Is>
struct repeat_impl<C, std::array<T, N>, index_sequence<Is...>> {
	typedef C<typename std::tuple_element<Is, std::array<T, N>>::type...> type;
};

template <typename T, size_t N, template <typename...> class C>
using repeat = repeat_impl<C, std::array<T, N>, make_index_sequence<N>>;


template <typename T1, typename T2, typename ... Ts>
struct is_decay_same : public std::conditional<is_decay_same<T1, T2>::value, is_decay_same<T1, Ts...>, std::false_type>::type {};

template <typename T1, typename T2>
struct is_decay_same<T1, T2> : public std::is_same<typename std::decay<T1>::type, typename std::decay<T2>::type>::type {};

template <typename T, typename ...Ts>
struct enable_if_decay_same : public std::enable_if<is_decay_same<T, Ts...>::value, typename std::decay<T>::type> {};


template <typename, typename> struct is_same : public false_type {};
template <typename T> struct is_same<T, T> : public true_type {};


template <typename, typename> struct is_return;
template <typename R, typename F, typename ... As>
struct is_return<R, F(As...)> : public is_same<R, typename std::result_of<F (As...)>::type>::type {};

}

#endif // __SEAL_TRAITS_H__

