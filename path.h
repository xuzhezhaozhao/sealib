
#ifndef _SEAL_PATH_H_
#define _SEAL_PATH_H_

#include "iters.h"

#include <algorithm>
#include <cassert>
#include <string>
#include <type_traits>
#include <unistd.h>


namespace sea {

static bool is_abs_path(const std::string &p) {
	return !p.empty() && p.front() == '/';
}

static std::string simplify_path(std::string &&path) {
	auto next = [&path] (std::string::iterator b) {
		if ( *b == '/' ) ++b;
		return ipair(b, find(b, path.end(), '/'));
	};
	bool abs = is_abs_path(path);

	auto b = path.begin(), w = path.begin();
	while ( b != path.end() ) {
		auto p = next(b);
		b = p.end();
		if ( p.empty() );
		else if ( p.size() == 1 && p.front() == '.' );
		else if ( p.size() == 2 && p.front() == '.' && p.back() == '.' ) {
			std::string::reverse_iterator r(w);
			r = find(r, path.rend(), '/');
			if ( r != path.rend() ) ++r;
			w = r.base();
		} else {
			if ( abs || w != path.begin() ) *w++ = '/';
			w = copy(p.begin(), p.end(), w);
		}
	}
	if ( !path.empty() && path.back() == '/' ) *w++ = '/';
	path.erase(w, path.end());
	if ( abs && path.empty() ) path.push_back('/');
	return move(path);
}

static std::string simplify_path(const std::string &p) {
	return simplify_path(std::string(p));
}


template <typename T1, typename T2, typename ... Ts> struct is_decay_same : public std::conditional<is_decay_same<T1, T2>::value, is_decay_same<T1, Ts...>, std::false_type>::type {};

template <typename T1, typename T2> struct is_decay_same<T1, T2> : public std::is_same<typename std::decay<T1>::type, typename std::decay<T2>::type>::type {};

template <typename T, typename ...Ts> struct check_decay_same : public std::enable_if<is_decay_same<T, Ts...>::value, typename std::decay<T>::type> {};


template <typename P1, typename P2> static
typename check_decay_same<std::string, P1, P2>::type
join_path_impl(P1 &&p1, P2 &&p2) {
	if ( p1.empty() || is_abs_path(p2) ) {
		return std::forward<P2>(p2);
	} else if ( p1.back() != '/' ) {
		return std::forward<P1>(p1) + '/' + std::forward<P2>(p2);
	}
	return std::forward<P1>(p1) + std::forward<P2>(p2);
}


template <typename T, typename U>
constexpr typename std::enable_if<is_decay_same<T, U>::value, T &&>::type
forward_as(typename std::remove_reference<T>::type& t) {
	return static_cast<T&&>(t);
}

template <typename T, typename U>
constexpr typename std::enable_if<!is_decay_same<T, U>::value, U>::type
forward_as(typename std::remove_reference<T>::type& t) {
	return U(t);
}

template <typename P1, typename P2> static
std::string join_path(P1 &&p1, P2 &&p2) {
	return join_path_impl(forward_as<P1, std::string>(p1), forward_as<P2, std::string>(p2));
}

template <typename P, typename ... Ps>
static std::string join_path(P &&p, Ps &&... ps) {
	return join_path(std::forward<P>(p), join_path(std::forward<Ps>(ps)...));
}

static std::string current_path() {
	char buf[1024];
	char *r = getcwd(buf, 1024);
	assert(r != nullptr);
	return r;
}

static std::string make_abs_path(const std::string &p) {
	return join_path(current_path(), p);
}

}

#endif

