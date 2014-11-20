
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

/// 定义一个bool常量, 默认false
template <bool V = false> struct bool_constant : public std::false_type {
	typedef bool_constant type;
};
/// 特化版本true, 该版本中定义了enable类型
template <> struct bool_constant<true> : public std::true_type {
	typedef int enable;
	typedef bool_constant type;
};
/// 定义sea空间的false_type和true_type
/// C++11 标准库中也有, 不过这个有点不一样, true_type多加了enable类型的定义
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

/**
 * generic版本的类型转换函数, 直接调用使用 static_cast 实现
 * 
 * ... 参数是为特化版本的转换函数设置的, 可以自动判断调用哪个类型转换函数
 */
template <typename T, typename F>
static constexpr T
type_cast_impl(F &&f, ...) {
	return static_cast<T>(f);
}

/**
 * 若F类型中定义了as<T>()模板方法, 则调用 as<T>方法进行类型转换
 * 
 * 关于 .template 的解释, 其作用类似与typename关键字
 *
 * [http://stackoverflow.com/questions/2105901/how-to-fix-expected-primary-expression-before-error-in-c-template-code]
 * When the name of a member template specialization appears after . or -> in a 
 * postfix-expression, or after nested-name-specifier in a qualified-id, and the 
 * postfix-expression or qualified-id explicitly depends on a template-parameter (14.6.2), 
 * the member template name must be prefixed by the keyword template. Otherwise
 * the name is assumed to name a non-template
 */
template <typename T, typename F>
static constexpr T
type_cast_impl(F &&f, typename std::add_pointer<decltype(std::declval<F>().template as<T>())>::type) {
	return std::forward<F>(f).template as<T>();
}

/**
 * 将类型F的变量转为类型T的变量. 
 * 
 * 当F中有as<T>()方法时用as<T>()实现, F中没有as<T>方法时就用 static_cast<T> 转换
 * 
 * F 变量可能为 space_t
 */
template <typename T, typename F>
static constexpr T
type_cast(F &&f) {
	/// nullptr 参数的作用是根据F中有无as<T>方法调用正确的重载函数
	return type_cast_impl<T>(std::forward<F>(f), nullptr);
}



/// 整数序列模板, size()方法取得序列大小
template <typename T, T ... Is> struct integer_sequence {
	typedef T value_type;
	typedef integer_sequence type;
	static constexpr size_t size() { return sizeof...(Is); }
};

/// 最后的type类型 为 integer_sequence<T, 0, 1, 2, ... , N - 1>
template <typename T, T N, T ... Is> struct make_iseq_impl {
	typedef typename std::conditional<N == 0, integer_sequence<T, Is...>, make_iseq_impl<T, N-1, N-1, Is...>>::type::type type;
};

/// 该类型为 integer_sequence<T, 0, 1, 2, ... , N - 1>
template <typename T, T N>
using make_integer_sequence = typename make_iseq_impl<T, N>::type;

template <size_t... Is>
using index_sequence = integer_sequence<size_t, Is...>;

/// 类型为 integer_sequence<size_t, 0, 1, 2, ... , N - 1>
template <size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;


/**
 * 下面的偏特化模板实现了将一种类型(T)的修饰(&, &&, const 修饰等)添加到另一种类型(U)上
 */
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


/// 判断decay之后的T1, T2, ...  的类型是不是都相同, 继承true_type 或 false_type
template <typename T1, typename T2, typename ... Ts>
struct is_decay_same : public std::conditional<is_decay_same<T1, T2>::value, is_decay_same<T1, Ts...>, std::false_type>::type {};

/// 判断decay之后的T1, T2类型是否相同, 继承true_type 或 false_type
template <typename T1, typename T2>
struct is_decay_same<T1, T2> : public std::is_same<typename std::decay<T1>::type, typename std::decay<T2>::type>::type {};

/// 若参数模板中所有参数decay之后类型都相同, 那么成员type为第一个参数decay之后的类型
template <typename T, typename ...Ts>
struct enable_if_decay_same : public std::enable_if<is_decay_same<T, Ts...>::value, typename std::decay<T>::type> {};


/// 注意: 这里继承的是sea::false_type, sea::true_type, 与std::下的有点不一样
/// true_type中加了enable的定义, false_type中没有
template <typename, typename> struct is_same : public false_type {};
template <typename T> struct is_same<T, T> : public true_type {};


template <typename, typename> struct is_return;
/// sea::is_same<>::type 是sea::true_type 或者 sea::false_type
/// 所以is_return 继承的是sea::true_type或者sea::false_type
/// 当 F(As...)返回值与R类型相同时, is_return继承true_type, 否则false_type
template <typename R, typename F, typename ... As>
struct is_return<R, F(As...)> : public is_same<R, typename std::result_of<F (As...)>::type>::type {};

}

#endif // __SEAL_TRAITS_H__

