
#ifndef __SEAL_CONFIGURE_H_
#define __SEAL_CONFIGURE_H_

#include "filepool.h"
#include "functor.h"
#include "hash.h"

#include <functional>
#include <string>

#include <cstdio>
#include <cstring>


namespace sea {

namespace config_impl {


static terminate_error unknown(const char *p) {
	std::string s;
	string_writer(s).format("unknown argument '%s'\n", p);
	return terminate_error(s);
}

static terminate_error unexpected(const std::string &n, const std::string &v, const hash_set<std::string> &e) {
	std::string s;
	string_writer w = {s};
	w("unexpected value '%s' of '%s'\ncandidates are:\n\t", v.data(), n.data());
	for (const std::string &c : e) {
		w(c)(' ');
	}
	w.nl();
	return terminate_error(s);
}

template <typename R>
static terminate_error unexpected(const std::string &n, const std::string &v, const hash_map<std::string, R> &e) {
	hash_set<std::string> s;
	for (auto &p : e) {
		s.insert(p.first);
	}
	return unexpected(n, v, s);
}



class base_argument {
public:
	bool valid = false;
	bool inited = false;
	bool append;
	std::string name;
	std::string argv;

	void assign(const std::string &v) {
		if ( append ) {
			argv.append(v).append(1, ' ');
		} else {
			argv = v;
		}
		inited = true;
	}
};

template <typename R, bool E> struct basic_argument : public base_argument {
	typedef R resource_type;
	typedef std::integral_constant<bool, E> has_expect;
};


inline namespace pub {

template <typename R> struct mapped_arg : public basic_argument<R, true> {
	R value;
	hash_map<std::string, R> expect;

	bool process() {
		auto f = expect.find(this->argv);
		if ( f == expect.end() ) {
			error(unexpected(this->name, this->argv, expect));
			return false;
		}
		value = f->second;
		return true;
	}
};

template <typename R> struct functor_arg : public basic_argument<R, false> {
	R value;
	std::function<bool (functor_arg &)> func;
	bool process() { return func(*this); };
};

template <typename = void> struct limited_arg : public basic_argument<void, true> {
	hash_set<std::string> expect;
	bool process() {
		if ( expect.count(argv) == 0 ) {
			error(unexpected(this->name, this->argv, expect));
			return false;
		}
		return true;
	}
};

template <typename = void> struct regular_arg : public basic_argument<void, false> {
	bool process() { return true; }
};

template <typename R, typename ... Ps>
mapped_arg<R> make_mapped(const std::string &n, const Ps &... ps) {
	mapped_arg<R> arg;
	arg.name = n;
	arg.append = false;
	auto f = map_inserter<hash_map<std::string, R>>(arg.expect);
	f(ps...);
	return arg;
}

template <typename R, typename F>
functor_arg<R> make_functor(const std::string &n, const F &f, bool app = false) {
	functor_arg<R> arg;
	arg.name = n;
	arg.append = app;
	arg.func = f;
	return arg;
}

template <typename ... Ps>
limited_arg<> make_limited(const std::string &n, const Ps &... ps) {
	limited_arg<> arg;
	arg.name = n;
	arg.append = false;
	arg.expect = {ps...};
	return arg;
}

inline regular_arg<> make_regular(const std::string &n, bool app = false) {
	regular_arg<> arg;
	arg.name = n;
	arg.append = app;
	return arg;
}

typedef functor_arg<FILE *> file_arg;

inline file_arg make_file_arg(const std::string &n, const char *m) {
	return make_functor<FILE *>(n, [m] (file_arg &arg) {
		return arg.value = file_pool::open(arg.argv.data(), m);
	});
}

typedef mapped_arg<bool> bool_arg;

inline bool_arg make_bool_arg(const std::string &n) {
	return make_mapped<bool>(n,
			"enable", true, "disable", false,
			"true", true, "false", false,
			"on", true, "off", false,
			"1", true, "0", false);
}

}

struct process_helper {
	// TODO c++14 use generic lambda
	template <typename A>
	int operator()(A &a) const {
		if ( a.inited ) {
			a.valid = a.process();
		}
		return 0;
	}
};

inline namespace pub {

template <typename ... As>
class configure_arguments {
private:
	std::tuple<As &...> _args;
	hash_map<std::string, std::reference_wrapper<base_argument>> _dict;

public:
	configure_arguments(As &... as): _args(as...) {
		auto m = [](base_argument &a) {
			return std::make_pair(a.name, std::ref(a));
		};
		auto r = reduce_constructor<hash_map<std::string, std::reference_wrapper<base_argument>>>();
		_dict = apply(map(_args, m), r);
	}

	void append(const std::string &k, const std::string &v) {
		auto f = _dict.find(k);
		if ( f == _dict.end() ) {
			error(unknown(k.data()));
		} else {
			f->second.get().assign(v);
		}
	}

	void parse_command_line(int argc, char *argv[]) {
		std::string key, val;
		for (int i = 1; i <= argc; ++i) {
			char *s = argv[i];
			if ( s[0] != '-' || s[1] == '\0' ) {
				error(unknown(s));
				continue;
			} else if ( s[1] == '-' ) {
				char *e = strchr(s, '=');
				if ( e ) {
					key = {s, e};
					val = e+1;
				} else {
					key = s;
					val = argv[++i];
				}
			} else {
				if ( s[2] == '\0' ) {
					key = s;
					val = argv[++i];
				} else {
					key = {s, s+2};
					val = s+2;
				}
			}
			append(key, val);
		}
	}

	void process() {
		process_helper m;
		map(_args, m);
	}

	seal_macro_only_move(configure_arguments)
};

template <typename ... As>
configure_arguments<As...> make_configure_arguments(As &... as) {
	return configure_arguments<As...>(as...);
}

}
}

using namespace config_impl::pub;

}

#endif // __SEAL_CONFIGURE_H_

