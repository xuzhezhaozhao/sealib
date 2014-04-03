
#ifndef __SEAL_MACRO_H__
#define __SEAL_MACRO_H__


#define seal_macro_non_copy(t)			\
	t(const t &) = delete;				\
	t &operator=(const t &) = delete;

#define seal_macro_only_move(t)			\
	seal_macro_non_copy(t)				\
	t(t &&) = default;					\
	t &operator=(t &&) = default;

#define seal_macro_def_has_elem(e)									\
	template <typename T, typename R> std::true_type 				\
	seal_has_##e##_helper_impl(decltype((R)&T::e));					\
	template <typename T, typename R> std::false_type				\
	seal_has_##e##_helper_impl(...);								\
	template <typename T, typename R>								\
	using seal_has_##e##_type_impl =								\
	decltype(seal_has_##e##_helper_impl<T, R>(nullptr));			\
	template <typename T, typename R> struct has_##e :				\
	public seal_has_##e##_type_impl<T, R> {};						\
	template <typename T, typename R>								\
	using enable_if_has_##e = std::enable_if<has_##e<T, R>::value, T>


#define likely(x)	__builtin_expect((x), true)
#define unlikely(x)	__builtin_expect((x), false)

#endif

