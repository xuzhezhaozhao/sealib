
#ifndef __SEAL_SPLIT_H__
#define __SEAL_SPLIT_H__

#include "iters.h"

#include <algorithm>
#include <string>
#include <unordered_map>


namespace sea {

class char_mask {
private:
	uint64_t _arr[4];

	static constexpr uint64_t trans(const char *s, size_t o) {
		return *s ? (set((size_t)*s, o) | trans(s+1, o)) : 0;
	}

	static constexpr uint64_t set(size_t i, size_t o) {
		return i / 64 == o ? ((uint64_t)1 << i % 64) : 0;
	}

	constexpr char_mask(uint64_t a, uint64_t b, uint64_t c, uint64_t d): _arr{a, b, c, d} {}

public:
	static constexpr char_mask make(const char *s) {
		return char_mask(trans(s, 0), trans(s, 1), trans(s, 2), trans(s, 3));
	}

	char_mask(const char *s): _arr{} {
		while ( *s ) set(*s++, true);
	}

	constexpr bool operator[](char c) const { return test(c); }
	constexpr bool operator()(char c) const { return test(c); }
	constexpr bool test(char c) const { return test((size_t)c); }
	constexpr bool test(size_t i) const { return _arr[i / 64] & ((uint64_t)1 << i % 64); }

	char_mask &set(char c, bool v) { return set((size_t)c, v); }
	char_mask &set(size_t i, bool v) {
		if ( v ) _arr[i / 64] |= ((uint64_t)1 << i % 64);
		else _arr[i / 64] &= ~((uint64_t)1 << i % 64);
		return *this;
	}

	void clear() { _arr[0] = _arr[1] = _arr[2] = _arr[3] = 0; }
};


class spliter {
public:
	typedef std::string::const_iterator str_iter;
	typedef iter_pair<str_iter> sub_str;

private:
	char_mask _sep;

public:
	constexpr spliter(): _sep(char_mask::make(" \t\n\r")) {}

	constexpr spliter(const char_mask &m): _sep(m) {}

	spliter(const char *s): _sep(s) {}

	template <typename __F>
	void split(const std::string &str, __F &&f) const {
		str_iter pos = str.begin();
		while ( pos != str.end() ) {
			sub_str s = next_token(pos, str.end());
			if ( s.begin() < s.end() ) {
				f(s);
			}
		}
	}

	std::vector<std::string> split(const std::string &str) const {
		std::vector<std::string> rst;
		split(str, [&rst](const sub_str &s) { rst.emplace_back(s.begin(), s.end()); });
		return std::move(rst);
	}

	sub_str next_token(str_iter &pos, str_iter end) const {
		str_iter i = std::find_if_not(pos, end, _sep);
		if ( i == end ) {
			pos = i;
			return sub_str(i, i);
		} else if ( *i == '"' || *i == '\'' ) {
			str_iter j = std::find(i + 1, end, *i);
			while ( j != end && *(j - 1) == '\\' ) {
				j = std::find(j + 1, end, *i);
			}
			pos = j == end ? j : j + 1;
			return sub_str(i + 1, j);
		} else {
			str_iter j = std::find_if(i + 1, end, _sep);
			pos = j;
			return sub_str(i, j);
		}
	}

};


class config_parser {
public:
	typedef spliter::str_iter str_iter;
	typedef spliter::sub_str sub_str;
	typedef std::unordered_map<std::string, std::string> dictionary;

private:
	spliter _spliter = char_mask::make(" \t\r\n,;=:");

public:
	template <typename __F>
	void parse(const std::string &str, __F &&f) const {
		str_iter ik = str.begin();
		auto sf = [] (char c) { return c == '=' || c == ':'; };
		while ( true ) {
			str_iter is = std::find_if(ik, str.end(), sf);
			if ( is == str.end() ) {
				break;
			}
			str_iter iv = is + 1;
			sub_str vp = _spliter.next_token(iv, str.end());
			while ( ik != is ) {
				sub_str kp = _spliter.next_token(ik, iv);
				if ( !kp.empty() ) {
					f(kp, vp);
				}
			}
			ik = iv;
		}
	}

	dictionary parse(const std::string &str) const {
		dictionary dict;
		parse(str, [&dict] (const sub_str &k, const sub_str &v) {
				dict.emplace(std::piecewise_construct, k.tuple(), v.tuple());
				});
		return std::move(dict);
	}
};


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

	rs_type get(const std::string &s) const {
		auto f = _dict.find(s);
		if ( f == _dict.end() ) {
			return rs_type();
		}
		return f->second;
	}

	template <typename __C, typename __F>
	void open(const __C &cc, __F &&f) {
		rs_dict_type vr;
		for (auto &kv : cc) {
			open(vr, kv.first, kv.second, f);
		}
	}

	template <typename __S, typename __F>
	void open(const std::string &str,
			__F &&f, const __S & set) {
		typedef config_parser::sub_str sub_str;
		const config_parser cp;
		rs_dict_type vr;
		cp.parse(str, [this, &vr, &set, &f] (const sub_str &k, const sub_str &v) {
				std::string ks = k.as<std::string>();
				std::string vs = v.as<std::string>();
				if ( set.count(ks) > 0 ) {
					open(vr, ks, vs, f);
				}
			});
	}

	template <typename __F>
	void close(__F &&f) {
		for (auto &r : _pool) {
			f(r);
		}
		_pool.clear();
	}

private:
	template <typename __F>
	void open(rs_dict_type &vr, const std::string &k, const std::string &v, __F &&f) {
		auto i = vr.find(v);
		if ( i == vr.end() ) {
			i = vr.emplace(v, f(v)).first;
			_pool.push_back(i->second);
		}
		_dict.emplace(k, i->second);
	}
};

}

#endif


