
#ifndef __SEAL_HASH_H__
#define __SEAL_HASH_H__

#include "incomplete.h"
#include "macro.h"

#include <type_traits>
#include <unordered_map>
#include <unordered_set>


namespace sea {
seal_macro_def_has_elem(hash_code);

template <typename T, typename = void>
struct hash {
	typedef size_t result_type;
	typedef T arg_type;
	size_t operator()(const arg_type &a) const noexcept {
		return a.hash_code();
	};
};

template <typename T, typename U>
struct hash<std::pair<T *, U *>> {
	typedef size_t result_type;
	typedef std::pair<T *, U *> arg_type;
	size_t operator()(const arg_type &a) const noexcept {
		size_t v1 = (size_t)(a.first);
		size_t v2 = (size_t)(a.second);
		static constexpr size_t S = sizeof(size_t) * 4;
		static constexpr size_t M = ((size_t)(1) << S) - 1;
		return (v1 & M) | (v2 << S);
	}
};

template <typename T>
struct hash<T, typename make_void<typename std::hash<T>::arg_type>::type> : public std::hash<T> {};


template <typename T, typename H = sea::hash<T>,
		 typename P = std::equal_to<T>, typename A = std::allocator<T>>
using hash_set = std::unordered_set<T, H, P, A>;

template <typename K, typename V, typename H = sea::hash<K>,
		 typename P = std::equal_to<K>, typename A = std::allocator<std::pair<K, V>>>
using hash_map = std::unordered_map<K, V, H, P, A>;

}

#endif

