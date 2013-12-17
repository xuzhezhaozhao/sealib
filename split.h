
#ifndef __SEAL_SPLIT_H__
#define __SEAL_SPLIT_H__

#include "iters.h"

#include <algorithm>
#include <bitset>
#include <string>
#include <unordered_map>


namespace sea {

class spliter {
public:
	typedef std::string::const_iterator str_iter;
	typedef iter_pair<str_iter> sub_str;

private:
	typedef std::bitset<256> separator;
	separator _sep;

public:
	spliter() = default;

	spliter(const char *s) {
		while ( *s ) {
			_sep[*s++] = true;
		}
	}

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
		auto sf = [this] (char c) { return _sep[c]; };
		str_iter i = std::find_if_not(pos, end, sf);
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
			str_iter j = std::find_if(i + 1, end, sf);
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
	spliter _spliter = {" \t\r\n,;=:"};

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
				if ( kp.begin() < kp.end() ) {
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

	template <typename __C, typename __F>
	void open(const __C &cc, __F &&f) {
		rs_dict_type vr;
		for (auto &kv : cc) {
			open(vr, kv.first, kv.second, f);
		}
	}

	template <typename __F>
	void open(const config_parser &cp, const std::string &str, __F &&f) {
		typedef config_parser::sub_str sub_str;
		rs_dict_type vr;
		cp.parse(str, [this, &vr, &f] (const sub_str &k, const sub_str &v) {
				open(vr, k.as<std::string>(), v.as<std::string>(), f);
				});
	}

	template <typename __F>
	void open(const std::string &str, __F &&f) {
		open(config_parser{}, str, f);
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


