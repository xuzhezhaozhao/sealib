
#ifndef _SEAL_PATH_H_
#define _SEAL_PATH_H_

#include <algorithm>
#include <cassert>
#include <string>
#include <type_traits>
#include <unistd.h>


namespace sea {

static bool is_abs_path(const std::string &p) {
	return !p.empty() && p.front() == '/';
}

static std::string simplify_path(std::string &&p) {
	typedef std::string::iterator iter;
	typedef std::reverse_iterator<iter> riter;
	iter i = p.begin(), j = p.begin();
	riter r = p.rend();
	while ( i != p.end() ) {
		if ( p.end() - i < 3 ) {
			j = std::copy(i, p.end(), j);
			break;
		} else if ( std::equal(i, i + 3, "/./") ) {
			i += 2;
		} else if ( p.end() - i < 4 ) {
			j = std::copy(i, p.end(), j);
			break;
		} else if ( std::equal(i, i + 4, "/../") ) {
			j = std::find(riter(j), r, '/').base();
			if ( *j == '/' ) {
				++j;
			}
			i += 4;
		} else {
			*j++ = *i++;
		}
	}
	p.erase(j, p.end());
	return std::move(p);
}

static std::string simplify_path(const std::string &p) {
	std::string s = p;
	return simplify_path(std::move(s));
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

