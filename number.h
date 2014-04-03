
#ifndef __SEAL_NUMBER_H__
#define __SEAL_NUMBER_H__

#include "dimension.h"
#include "incomplete.h"
#include "macro.h"

#include <cmath>
#include <cstdlib>
#include <type_traits>


namespace sea {

class writer;

template <typename T, typename D = dimensions::unit, typename R = std::integral_constant<int, 0>, typename G = void> class number;


template <typename>
struct is_number : public std::false_type {};
template <typename T, typename D, typename R, typename G>
struct is_number<number<T, D, R, G>> : public std::true_type {};

template <typename N, typename T = N>
using enable_if_is_number = std::enable_if<is_number<N>::value, T>;



template <bool V, typename T>
struct with_flag {
	typedef std::integral_constant<bool, V> flag_type;
	static constexpr bool value = V;
	typedef T type;
};

template <typename T>
using with_true_flag = with_flag<true, T>;
template <typename T>
using with_false_flag = with_flag<false, T>;

namespace operators {

template <typename G1, typename G2>
struct tag_same : public with_flag<std::is_same<G1, G2>::value, G1> {};


// operate result type
struct operate_error : public with_false_flag<number<int>> {};

template <typename N1, typename N2, typename Op>
struct operate_impl {
private:
	typedef typename N1::value_type n1_vt;
	typedef typename N2::value_type n2_vt;
	typedef typename N1::dimension_type n1_dt;
	typedef typename N2::dimension_type n2_dt;
	typedef typename N1::ratio_type n1_rt;
	typedef typename N2::ratio_type n2_rt;
	typedef typename N1::tag_type n1_gt;
	typedef typename N2::tag_type n2_gt;
public:
	static constexpr bool value =
		Op::template get_vt<n1_vt, n2_vt>::value &&
		Op::template get_dt<n1_dt, n2_dt>::value &&
		Op::template get_rt<n1_rt, n2_rt>::value &&
		Op::template get_gt<n1_gt, n2_gt>::value;

	typedef std::integral_constant<bool, value> flag_type;
	typedef number<
		typename Op::template get_vt<n1_vt, n2_vt>::type,
		typename Op::template get_dt<n1_dt, n2_dt>::type,
		typename Op::template get_rt<n1_rt, n2_rt>::type,
		typename Op::template get_gt<n1_gt, n2_gt>::type
		> type;
};

template <typename N1, typename N2, typename Op>
struct operate : std::conditional<is_number<N1>::value && is_number<N2>::value, operate_impl<N1, N2, Op>, operate_error>::type {};

#define macro_def_tag_operate(xx)														\
	template <typename G1, typename G2, typename = void>								\
	struct tag_##xx : public tag_same<G1, G2> {};										\
	template <typename G1, typename G2>													\
	struct tag_##xx<G1, G2, typename make_void<typename G1::template xx<G2>>::type>		\
	: public G1::template xx<G2> {}

	macro_def_tag_operate(addsub);
	macro_def_tag_operate(muldiv);
	macro_def_tag_operate(compare);
	macro_def_tag_operate(iaddsub);
	macro_def_tag_operate(imuldiv);
	macro_def_tag_operate(assign);

#undef macro_def_tag_operate

template <template <typename ...> class GG>
struct addsub_compare_impl {
	template <typename T1, typename T2>
	using get_vt = with_true_flag<typename std::common_type<T1, T2>::type>;

	template <typename D1, typename D2>
	using get_dt = with_flag<std::is_same<D1, D2>::value, D1>;

	template <typename R1, typename R2>
	using get_rt = with_true_flag<typename std::conditional<(R1::value < R2::value), R1, R2>::type>;

	template <typename G1, typename G2>
	using get_gt = GG<G1, G2>;
};

typedef addsub_compare_impl<tag_addsub> add;
typedef addsub_compare_impl<tag_addsub> sub;
typedef addsub_compare_impl<tag_compare> compare;


template <template <typename ...> class GG>
struct iaddsub_assign_impl {
	template <typename T1, typename T2>
	using get_vt = with_true_flag<T1>;

	template <typename D1, typename D2>
	using get_dt = with_flag<std::is_same<D1, D2>::value, D1>;

	template <typename R1, typename R2>
	using get_rt = with_true_flag<R1>;

	template <typename G1, typename G2>
	using get_gt = GG<G1, G2>;
};

typedef iaddsub_assign_impl<tag_iaddsub> iadd;
typedef iaddsub_assign_impl<tag_iaddsub> isub;
typedef iaddsub_assign_impl<tag_assign> assign;


struct mul {
	template <typename T1, typename T2>
	using get_vt = with_true_flag<typename std::common_type<T1, T2>::type>;

	template <typename D1, typename D2>
	using get_dt = with_true_flag<typename dimensions::mul<D1, D2>::type>;

	template <typename R1, typename R2>
	using get_rt = with_true_flag<std::integral_constant<int, R1::value + R2::value>>;

	template <typename G1, typename G2>
	using get_gt = tag_muldiv<G1, G2>;
};

struct div {
	template <typename T1, typename T2>
	using get_vt = with_true_flag<typename std::common_type<T1, T2>::type>;

	template <typename D1, typename D2>
	using get_dt = with_true_flag<typename dimensions::div<D1, D2>::type>;

	template <typename R1, typename R2>
	using get_rt = with_true_flag<std::integral_constant<int, R1::value - R2::value>>;

	template <typename G1, typename G2>
	using get_gt = tag_muldiv<G1, G2>;
};

typedef struct {
	template <typename T1, typename T2>
	using get_vt = with_true_flag<T1>;

	template <typename D1, typename D2>
	using get_dt = with_flag<std::is_same<D2, dimensions::unit>::value, dimensions::unit>;

	template <typename R1, typename R2>
	using get_rt = with_true_flag<std::integral_constant<int, 0>>;

	template <typename G1, typename G2>
	using get_gt = tag_imuldiv<G1, G2>;
} imul, idiv;


#define macro_def_do_operate(xx, o, r)							\
	seal_macro_def_has_elem(do_##xx);							\
	template <typename G, typename T>							\
	static constexpr r do_##xx(T u, T v, std::true_type)					\
	{ return G::do_##xx(u, v); }								\
	template <typename  , typename T>							\
	static constexpr r do_##xx(T u, T v, std::false_type)					\
	{ return u o v; }											\
	template <typename N, typename T>							\
	static constexpr r do_##xx(T u, T v) {								\
		typedef typename N::tag_type gt;						\
		return do_##xx<gt>(u, v, has_do_##xx<gt, bool (*)(T, T)>());	\
	}

macro_def_do_operate(lt, <,  bool)
macro_def_do_operate(eq, ==, bool)
macro_def_do_operate(le, <=, bool)

#undef macro_def_do_operate


template <typename T, int R>
struct power {
	static constexpr T value = std::conditional<(R < 0), power<T, -R>, typename std::conditional<(R > 0), power<T, R-1>, std::integral_constant<int, 1>>::type>::type::value * (R > 0 ? 10 : 1);
	static constexpr T calc(T v) {
		return R > 0 ? v * value : v / value;
	}
};

template <typename T, typename F>
static constexpr T castto(F v) {
	return std::is_floating_point<F>::value && !std::is_floating_point<T>::value ? (T)round(v) : (T)v;
}

}


template <typename __T, typename __D, typename __R, typename __G>
class number {
public:
	typedef number type;
	typedef __T value_type;
	typedef __D dimension_type;
	typedef __R ratio_type;
	typedef __G tag_type;

private:
	value_type _v;

	template <typename N, typename Op>
	using operate = operators::operate<number, N, Op>;

public:
	// ctor and assign
	number() = default;
	explicit constexpr number(value_type v): _v(v) {}

	template <typename N, typename O = operate<N, operators::assign>>
	constexpr number(const N &n): _v(n.as<number>().val()) {
		static_assert(O::value, "cannot construct");
	}

	template <typename N, typename O = operate<N, operators::assign>>
	number &operator=(const N &n) {
		static_assert(O::value, "cannot assign");
		_v = n.as<number>().val();
		return *this;
	}


	// compare
	template <typename N, typename O = operate<N, operators::compare>>
	constexpr bool operator<(const N &n) const {
		static_assert(O::value, "cannot compare");
		typedef typename O::type nt;
		return operators::do_lt<nt>(as<nt>().val(), n.as<nt>().val());
	}

	template <typename N, typename O = operate<N, operators::compare>>
	constexpr bool operator>(const N &n) const {
		static_assert(O::value, "cannot compare");
		return n < *this;
	}

	template <typename N, typename O = operate<N, operators::compare>>
	constexpr bool operator==(const N &n) const {
		static_assert(O::value, "cannot compare");
		typedef typename O::type nt;
		return operators::do_eq<nt>(as<nt>().val(), n.as<nt>().val());
	}

	template <typename N, typename O = operate<N, operators::compare>>
	constexpr bool operator!=(const N &n) const {
		static_assert(O::value, "cannot compare");
		return !(*this == n);
	}

	template <typename N, typename O = operate<N, operators::compare>>
	constexpr bool operator<=(const N &n) const {
		static_assert(O::value, "cannot compare");
		typedef typename O::type nt;
		return operators::do_le<nt>(as<nt>().val(), n.as<nt>().val());
	}

	template <typename N, typename O = operate<N, operators::compare>>
	constexpr bool operator>=(const N &n) const {
		static_assert(O::value, "cannot compare");
		return n <= *this;;
	}


	// add
	template <typename N, typename O = operate<N, operators::add>>
	constexpr typename O::type operator+(const N &n) const {
		static_assert(O::value, "cannot add");
		typedef typename O::type nt;
		return nt(as<nt>().val() + n.as<nt>().val());
	}

	template <typename N, typename O = operate<N, operators::iadd>>
	number &operator+=(const N &n) {
		static_assert(O::value, "cannot iadd");
		_v += n.as<number>().val();
		return *this;
	}

	number &operator++() { ++_v; return *this; }
	number operator++(int) { return number(_v++); }
	constexpr number operator+() const { return number(+_v); }


	// sub
	template <typename N, typename O = operate<N, operators::sub>>
	constexpr typename O::type operator-(const N &n) const {
		static_assert(O::value, "cannot sub");
		typedef typename O::type nt;
		return nt(as<nt>().val() - n.as<nt>().val());
	}

	template <typename N, typename O = operate<N, operators::isub>>
	number &operator-=(const N &n) {
		static_assert(O::value, "cannot isub");
		_v -= n.as<number>().val();
		return *this;
	}

	number &operator--() { --_v; return *this; }
	number operator--(int) { return number(_v--); }
	constexpr number operator-() const { return number(-_v); }


	// mul
	template <typename N, typename O = operate<typename enable_if_is_number<N>::type, operators::mul>>
	constexpr typename O::type operator*(const N &n) const {
		static_assert(O::value, "cannot mul");
		typedef typename O::type nt;
		return nt(val() * n.val());
	}

	template <typename N, typename O = operate<typename enable_if_is_number<N>::type, operators::imul>>
	number &operator*=(const N &n) {
		static_assert(O::value, "cannot imul");
		typedef typename O::type nt;
		_v = operators::castto<value_type>(_v * n.as<nt>.val());
		return *this;
	}

	template <typename N, typename = typename std::enable_if<std::is_arithmetic<N>::value>::type>
	constexpr number operator*(N n) const {
		return number(operators::castto<value_type>(val() * n));
	}

	template <typename N, typename = typename std::enable_if<std::is_arithmetic<N>::value>::type>
	number &operator*=(N n) {
		_v = operators::castto<value_type>(_v * n);
		return *this;
	}


	// div
	template <typename N, typename O = operate<typename enable_if_is_number<N>::type, operators::div>>
	constexpr typename O::type operator/(const N &n) const {
		static_assert(O::value, "cannot div");
		typedef typename O::type nt;
		return nt(val() / n.val());
	}

	template <typename N, typename O = operate<typename enable_if_is_number<N>::type, operators::idiv>>
	number &operator/=(const N &n) {
		static_assert(O::value, "cannot idiv");
		typedef typename O::type nt;
		_v = operators::castto<value_type>(_v / n.as<nt>.val());
		return *this;
	}

	template <typename N, typename = typename std::enable_if<std::is_arithmetic<N>::value>::type>
	constexpr number operator/(N n) const {
		return number(operators::castto<value_type>(val() / n));
	}

	template <typename N, typename = typename std::enable_if<std::is_arithmetic<N>::value>::type>
	number &operator/=(N n) {
		_v = operators::castto<value_type>(_v / n);
		return *this;
	}

	// convert
	template <typename N>
	constexpr typename enable_if_is_number<N>::type
	as() const {
		typedef std::integral_constant<int, ratio_type::value - N::ratio_type::value> rt;
		typedef typename std::common_type<value_type, typename N::value_type>::type tt;
		return N(operators::castto<typename N::value_type>(operators::power<tt, rt::value>::calc(val())));
	}

	constexpr value_type val() const { return _v; }
	constexpr value_type value() const { return val(); }

	void write_to(writer &) const;
};


template <typename N>
static constexpr typename enable_if_is_number<N>::type
abs(N n) { return std::is_signed<typename N::value_type>::value ? N(std::abs(n.val())) : n; }

}

#endif

