
#ifndef __SEAL_INCOMPLETE_H__
#define __SEAL_INCOMPLETE_H__

#include <cstdint>

namespace sea {

template <typename  T> struct incomplete_t;
template <intmax_t  V> struct incomplete_i;
template <uintmax_t V> struct incomplete_u;

struct empty {};

}

#endif


