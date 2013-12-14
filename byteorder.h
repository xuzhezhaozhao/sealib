
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace sea {

template <typename __UInt>
static constexpr __UInt swap_byte_order_impl(__UInt v, __UInt r, size_t i) {
	return i == 0 ? r : swap_byte_order_impl((__UInt)(v >> 8), (__UInt)(r | ((v & 0xff) << ((i - 1) * 8))), i - 1);
}

template <typename __Int>
static constexpr __Int swap_byte_order(__Int v) {
	static_assert(std::is_integral<__Int>::value, "parameter must be an integer");
	typedef typename std::make_unsigned<__Int>::type __UInt;
	return (__Int)swap_byte_order_impl((__UInt)v, (__UInt)0, sizeof(__UInt));
}

}

static_assert(sea::swap_byte_order((int8_t)0x01) == 0x01, "8");
static_assert(sea::swap_byte_order((int16_t)0x0102) == 0x0201, "16");
static_assert(sea::swap_byte_order(0x01020304) == 0x04030201, "32");
static_assert(sea::swap_byte_order(0x0102030405060708) == 0x0807060504030201, "64");

