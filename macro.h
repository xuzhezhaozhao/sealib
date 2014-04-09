
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
	template <typename T, typename R>								\
	struct has_##e##_impl {											\
		template <typename A, typename B> static std::true_type 	\
		helper(decltype((B)&A::e));									\
		template <typename A, typename B> static std::false_type	\
		helper(...);												\
		typedef decltype(helper<T, R>(nullptr)) type;				\
	};																\
	template <typename T, typename R> struct has_##e :				\
	public has_##e##_impl<T, R>::type {};							\
	template <typename T, typename R>								\
	using enable_if_has_##e = std::enable_if<has_##e<T, R>::value, T>


#define likely(x)	__builtin_expect((x), true)
#define unlikely(x)	__builtin_expect((x), false)

#endif

