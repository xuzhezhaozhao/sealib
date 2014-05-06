
#ifndef __SEAL_CONFIGURE_H_
#define __SEAL_CONFIGURE_H_

#include "filepool.h"
#include "functor.h"
#include "hash.h"
#include "iters.h"

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


template <typename S>
static terminate_error unexpected(const std::string &n, const std::string &v, const S &e) {
	std::string s;
	string_writer w = {s};
	w("unexpected value '%s' of '%s'\ncandidates are:\n\t", v.data(), n.data());
	for (const std::string &c : e) {
		w(c)(' ');
	}
	w.nl();
	return terminate_error(s);
}


class base_argument {
protected:
	std::string _name, _argv;
	bool _init = false, _valid = false, _app;

public:
	base_argument(const std::string &n, bool a): _name(n), _app(a) {}

	const std::string name() const { return _name; }
	const std::string argv() const { return _argv; }
	bool is_valid() const { return _valid; }

	base_argument &assign(const std::string &v) {
		if ( _app ) {
			_argv.append(v).append(1, ' ');
		} else {
			_argv = v;
		}
		_init = true;
		return *this;
	}

	base_argument &process() {
		_valid = _init && process_impl();
		return *this;
	}

	virtual bool process_impl() = 0;
	virtual ~base_argument() = default;
};

template <typename R> struct mapped_argument : public base_argument {
private:
	R &_val;
	hash_map<std::string, R> _expect;

public:
	typedef base_argument base_type;
	typedef R resource_type;

	template <typename M>
	mapped_argument(const std::string &n, R &v, M &&m):
		base_type(n, false), _val(v), _expect(std::forward<M>(m)) {}

	bool process_impl() override {
		auto f = _expect.find(_argv);
		if ( f == _expect.end() ) {
			error(unexpected(_name, _argv, key_cview(_expect)));
			return false;
		}
		_val = f->second;
		return true;
	}

	const resource_type &value() const { return _val; }
	resource_type &value() { return _val; }
};

template <typename R, typename F> struct functor_argument : public base_argument {
private:
	R &_val;
	F _func;

public:
	typedef base_argument base_type;
	typedef R resource_type;

	template <typename G>
	functor_argument(const std::string &n, R &v, G &&g, bool a = false):
		base_type(n, a), _val(v), _func(std::forward<G>(g)) {}

	bool process_impl() override { return _func(*this); }

	const resource_type &value() const { return _val; }
	resource_type &value() { return _val; }
};

template <typename R = std::string> struct regular_argument : public base_argument {
private:
	R &_val;

public:
	typedef base_argument base_type;
	typedef R resource_type;

	regular_argument(const std::string &n, R &v, bool a = false):
		base_type(n, a), _val(v) {}

	bool process_impl() override {
		_val = _argv;
		return true;
	}

	const resource_type &value() const { return _val; }
	resource_type &value() { return _val; }
};

struct file_opener {
	const char *m;
	bool operator()(functor_argument<FILE *, file_opener> &arg) const {
		return (arg.value() = file_pool::open(arg.argv().data(), m));
	}
};

struct value_convertor {
	template <typename R>
	bool operator()(functor_argument<R, value_convertor> &arg) const {
		arg.value() = convert(arg.argv().data(), R());
		return true;
	}

	int convert(const char *s, int) const { return atoi(s); }
	long long convert(const char *s, long long) const { return atoll(s); }
	double convert(const char *s, double) const { return atof(s); }
};

inline namespace pub {

template <typename R, typename ... Ps>
mapped_argument<R>
make_mapped_arg(const std::string &n, R &v, Ps &&... ps) {
	hash_map<std::string, R> m;
	auto f = map_inserter<hash_map<std::string, R>>(m);
	f(std::forward<Ps>(ps)...);
	return mapped_argument<R>(n, v, std::move(m));
}

template <typename R, typename F>
functor_argument<R, typename std::decay<F>::type>
make_functor_arg(const std::string &n, R &v, F &&f, bool app = false) {
	typedef functor_argument<R, typename std::decay<F>::type> arg_type;
	return arg_type(n, v, std::forward<F>(f), app);
}

template <typename R>
regular_argument<R> make_regular_arg(const std::string &n, R &v, bool app = false) {
	return regular_argument<R>(n, v, app);
}

inline functor_argument<FILE *, file_opener>
make_file_arg(const std::string &n, FILE *&f, const char *m) {
	return make_functor_arg(n, f, file_opener{m});
}

template <typename R>
functor_argument<R, value_convertor> make_number_arg(const std::string &n, R &v) {
	return make_functor_arg(n, v, value_convertor());
}

inline mapped_argument<bool> make_bool_arg(const std::string &n, bool &v) {
	return make_mapped_arg(n, v,
			"enable", true, "disable", false,
			"true", true, "false", false,
			"on", true, "off", false,
			"1", true, "0", false);
}


class configure_arguments {
private:
	typedef std::reference_wrapper<base_argument> ref_type;

	std::vector<ref_type> _args;
	hash_map<std::string, ref_type> _dict;

public:
	configure_arguments(std::initializer_list<ref_type> ls): _args(ls) {
		for (ref_type r : ls) {
			_dict.insert({r.get().name(), r});
		}
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
		for (int i = 1; i < argc; ++i) {
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
					val = ++i == argc ? "" : argv[i];
				}
			} else {
				if ( s[2] == '\0' ) {
					key = s;
					val = ++i == argc ? "" : argv[i];
				} else {
					key = {s, s+2};
					val = s+2;
				}
			}
			append(key, val);
		}
	}

	void process() {
		for (base_argument &a : _args) {
			a.process();
		}
	}

	seal_macro_only_move(configure_arguments)
};

}

}

using namespace config_impl::pub;

}

#endif // __SEAL_CONFIGURE_H_

