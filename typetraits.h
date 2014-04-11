
#ifndef __SEAL_TRAITS_H__
#define __SEAL_TRAITS_H__

#include <cstdint>
#include <type_traits>
#include <utility>


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
type_cast_impl(F &&f, decltype(&std::declval<F>().template as<T>())) {
	return std::forward<f>().template as<T>();
}

template <typename T, typename F>
static constexpr T
type_cast(F &&f) {
	return type_cast_impl<T>(std::forward<F>(f), nullptr);
}

}

#endif // __SEAL_TRAITS_H__

