
#ifndef __SEAL_OPT_H__
#define __SEAL_OPT_H__

#include "iters.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <unordered_map>


namespace sea {

namespace config_impl {

typedef std::string::const_iterator str_iter;
typedef iter_pair<str_iter> sub_str;

static sub_str get_token(str_iter &pos, str_iter end) {
	auto sep = [] (char c) { return strchr(" \b\t\n\r,;:=", c); };
	str_iter i = std::find_if_not(pos, end, sep);
	if ( i == end ) {
		pos = i;
		return sub_str(i, i);
	} else if ( *i == '"' || *i == '\'' ) {
		str_iter j = std::find(i + 1, end, *i);
		while ( j != end && *(j - 1) == '/' ) {
			j = std::find(j + 1, end, *i);
		}
		pos = j == end ? j : j + 1;
		return sub_str(i + 1, j);
	} else {
		str_iter j = std::find_if(i + 1, end, sep);
		pos = j;
		return sub_str(i, j);
	}
}

template <typename __F>
static void split(const std::string &str, __F &&f) {
	std::vector<std::string> result;
	str_iter pos = str.begin();
	while ( pos != str.end() ) {
		sub_str s = get_token(pos, str.end());
		if ( s.begin() < s.end() ) {
			f(s);
		}
	}
}

static std::vector<std::string> split(const std::string &str) {
	std::vector<std::string> rst;
	split(str, [&rst](const sub_str &s) { rst.emplace_back(s.begin(), s.end()); });
	return std::move(rst);
}


typedef std::unordered_map<std::string, std::string> dictionary;

template <typename __F>
void parse_config(const std::string &str, __F &&f) {
	str_iter ik = str.begin();
	auto sep = [] (char c) { return c == '=' || c == ':'; };
	while ( true ) {
		str_iter is = std::find_if(ik, str.end(), sep);
		if ( is == str.end() ) {
			break;
		}
		str_iter iv = is + 1;
		sub_str vp = get_token(iv, str.end());
		while ( ik != is ) {
			sub_str kp = get_token(ik, iv);
			if ( kp.begin() < kp.end() ) {
				f(kp, vp);
			}
		}
		ik = iv;
	}
}

dictionary parse_config(const std::string &str) {
	dictionary dict;
	parse_config(str, [&dict] (const sub_str &k, const sub_str &v) {
			dict.emplace(std::piecewise_construct, k.tuple(), v.tuple());
			});
	return std::move(dict);
}


template <typename __T>
class config_resource {
public:
	typedef __T rs_type;
	typedef std::unordered_map<std::string, rs_type> rs_dict_type;
	typedef std::vector<rs_type> rs_pool_type;
	typedef config_resource<rs_type> type;

private:
	rs_dict_type _dict;
	rs_pool_type _pool;

public:
	const rs_dict_type &dict() const { return _dict; }

	config_resource() = default;

	template <typename __C, typename __F>
	void open(const __C &cc, __F f) {
		opener<__F> o = {*this, f};
		for (auto &kv : cc) {
			o(kv.first, kv.second);
		}
	}

	template <typename __F>
	class opener {
	private:
		rs_dict_type _vr;
		type &_cr;
		__F _f;
	public:
		opener(type &cr, __F f): _cr(cr), _f(f) {
			assert(_cr._pool.empty());
		}
		opener &open(const std::string &k, const std::string &v) {
			auto i = _vr.find(v);
			if ( i == _vr.end() ) {
				i = _vr.emplace(v, _f(v)).first;
				_cr._pool.push_back(i->second);
			}
			_cr._dict.emplace(k, i->second);
			return *this;
		}
		opener &operator()(const std::string &k, const std::string &v) { return open(k, v); }
		opener &operator()(const sub_str &k, const sub_str &v) {
			return open(k.as<std::string>(), v.as<std::string>());
		}
		opener &operator++() { return *this; }
		opener &operator++(int) { return *this; }
		opener &operator*() { return *this; }

		opener &operator=(const std::pair<std::string, std::string> &p) {
			return open(p.first, p.second);
		}
		opener &operator=(const std::pair<sub_str, sub_str> &p) {
			return open(p.first.as<std::string>(), p.second.as<std::string>());
		}

		opener(const opener &) = delete;
		opener &operator=(const opener &) = delete;
		opener(opener &&) = default;
	};

	template <typename __F>
	opener<__F> get_opener(__F f) {
		return {*this, f};
	}

	template <typename __F>
	void close(__F f) {
		for (auto &r : _pool) {
			f(r);
		}
		_pool.clear();
	}
};

}

using config_impl::config_resource;
using config_impl::parse_config;
using config_impl::split;
using config_impl::sub_str;

}

#endif


