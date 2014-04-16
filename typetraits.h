
#ifndef __SEAL_TRAITS_H__
#define __SEAL_TRAITS_H__

#include <type_traits>
#include <utility>

#include <cstddef>
#include <cstdint>


namespace sea {

template <typename  T> struct incomplete_t;
template <intmax_t  V> struct incomplete_i;
template <uintmax_t V> struct incomplete_u;

struct empty {};


template <typename>
struct make_void {
	typedef void type;
};

template <bool>
struct always_void {
	typedef void type;
};


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

}

#endif // __SEAL_TRAITS_H__

