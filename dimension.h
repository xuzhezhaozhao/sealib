
#ifndef __SEAL_DIMENSION_H__
#define __SEAL_DIMENSION_H__

#include <type_traits>


namespace sea {

namespace dimensions {

template <typename __D>
struct is_dimension : public std::false_type {};


template <int __M, int __KG, int __S, int __A, int __K, int __ML, int __CD>
struct dimension {
public:
	typedef dimension type;
	typedef std::integral_constant<int, __M > meter_type;
	typedef std::integral_constant<int, __KG> kilogram_type;
	typedef std::integral_constant<int, __S > second_type;
	typedef std::integral_constant<int, __A > ampere_type;
	typedef std::integral_constant<int, __K > kelvin_type;
	typedef std::integral_constant<int, __ML> mole_type;
	typedef std::integral_constant<int, __CD> candela_type;

	static constexpr int meter = __M;
	static constexpr int kilogram = __KG;
	static constexpr int second = __S;
	static constexpr int ampere = __A;
	static constexpr int kelvin = __K;
	static constexpr int mole = __ML;
	static constexpr int candela = __CD;
};


template <int ... __V>
struct is_dimension<dimension<__V...>> : public std::true_type {};


typedef dimension<0, 0, 0, 0, 0, 0, 0> unit;
typedef dimension<1, 0, 0, 0, 0, 0, 0> meter;
typedef dimension<0, 1, 0, 0, 0, 0, 0> kilogram;
typedef dimension<0, 0, 1, 0, 0, 0, 0> second;
typedef dimension<0, 0, 0, 1, 0, 0, 0> ampere;
typedef dimension<0, 0, 0, 0, 1, 0, 0> kelvin;
typedef dimension<0, 0, 0, 0, 0, 1, 0> mole;
typedef dimension<0, 0, 0, 0, 0, 0, 1> candela;


template <typename D1, typename D2, typename ... Ds>
struct mul {
	typedef typename mul<typename mul<D1, D1>::type, Ds...>::type type;
};

template <typename D1, typename D2>
struct mul<D1, D2> {
	typedef dimension<
		D1::meter    + D2::meter   ,
		D1::kilogram + D2::kilogram,
		D1::second   + D2::second  ,
		D1::ampere   + D2::ampere  ,
		D1::kelvin   + D2::kelvin  ,
		D1::mole     + D2::mole    ,
		D1::candela  + D2::candela> type;
};

template <typename D1, typename D2, typename ... Ds>
struct div {
	typedef typename div<typename div<D1, D1>::type, Ds...>::type type;
};

template <typename D1, typename D2>
struct div<D1, D2> {
	typedef dimension<
		D1::meter    - D2::meter   ,
		D1::kilogram - D2::kilogram,
		D1::second   - D2::second  ,
		D1::ampere   - D2::ampere  ,
		D1::kelvin   - D2::kelvin  ,
		D1::mole     - D2::mole    ,
		D1::candela  - D2::candela> type;
};

}
}

#endif

