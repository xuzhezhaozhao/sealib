
#ifndef _SEAL_PATH_H_
#define _SEAL_PATH_H_

#include "iters.h"
#include "typetraits.h"

#include <algorithm>
#include <cassert>
#include <string>
#include <unistd.h>


namespace sea {

namespace path {

/// 判断路径 p 是否是绝对路径, 非空且以 '/' 开头就是绝对路径
static bool is_abs(const std::string &p) {
	return !p.empty() && p.front() == '/';
}

static std::string simplify(std::string &&path) {
	auto next = [&path] (std::string::iterator b) {
		if ( *b == '/' ) ++b;
		return ipair(b, find(b, path.end(), '/'));
	};
	bool abs = is_abs(path);

	auto b = path.begin(), w = path.begin();
	while ( b != path.end() ) {
		auto p = next(b);
		b = p.end();
		if ( p.empty() ) {
		} else if ( p.size() == 1 && p.front() == '.' ) {
		} else if ( p.size() == 2 && p.front() == '.' && p.back() == '.' ) {
			std::string::reverse_iterator r(w);
			r = find(r, path.rend(), '/');
			if ( r != path.rend() ) w = (++r).base();
			else if ( !abs && w == path.begin() ) w = copy(p.begin(), p.end(), w);
			else w = path.begin();
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

static std::string simplify(const std::string &p) {
	return simplify(std::string(p));
}


/// P1, P2类型decay之后应该都是string类型
/// 若p1为空或p2为绝对路径, 就返回该路径, 否则将p1和p2路径拼接起来返回
/// 如 p1 rwcap , p2 src, 返回 rwcap/src
template <typename P1, typename P2> static
typename enable_if_decay_same<std::string, P1, P2>::type
join_impl(P1 &&p1, P2 &&p2) {
	if ( p1.empty() || is_abs(p2) ) {	/// p1空或者p2为绝对路径, 直接返回p2
		return std::forward<P2>(p2);
	} else if ( p1.back() != '/' ) {
		return std::forward<P1>(p1) + '/' + std::forward<P2>(p2);
	}
	return std::forward<P1>(p1) + std::forward<P2>(p2);
}


/// 若T和U decay之后的类型相同, perfect forward 变量t即可
template <typename T, typename U>
constexpr typename std::enable_if<is_decay_same<T, U>::value, T &&>::type
forward_as(typename std::remove_reference<T>::type& t) {
	return static_cast<T&&>(t);
}

/// 若T和U decay之后的类型不同, 将变量t转为类型U并返回转换后的对象U
/// 比如T为char *字符串, U为std::string
template <typename T, typename U>
constexpr typename std::enable_if<!is_decay_same<T, U>::value, U>::type
forward_as(typename std::remove_reference<T>::type& t) {
	return U(t);
}

/// p1, p2为字符串, 可以是C风格的, 也可以是string类型的
/// 若p1为空或p2为绝对路径, 就返回该路径, 否则将p1和p2路径拼接起来返回
/// 如 p1 rwcap , p2 src, 返回 rwcap/src
template <typename P1, typename P2> static
std::string join(P1 &&p1, P2 &&p2) {
	return join_impl(forward_as<P1, std::string>(p1), forward_as<P2, std::string>(p2));
}

/// 将多个字符串路径拼接起来
template <typename P, typename ... Ps>
static std::string join(P &&p, Ps &&... ps) {
	return join(std::forward<P>(p), join(std::forward<Ps>(ps)...));
}

/// 获取当前的工作目录, 返回string类型的字符串
static std::string current() {
	char buf[1024];
	char *r = getcwd(buf, 1024);
	assert(r != nullptr);
	return r;
}

/// 将相对路径p转换成绝对路径
static std::string make_abs(const std::string &p) {
	return join(current(), p);
}

}
}

#endif

