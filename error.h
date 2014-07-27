
#ifndef __SEAL_ERROR_H__
#define __SEAL_ERROR_H__

#include "threads.h"
#include "typetraits.h"
#include "writer.h"

#include <stdexcept>
#include <typeindex>
#include <exception>
#include <unordered_map>
#include <utility>


namespace sea {

enum class error_type {BASIC, WARNING, FATAL};


struct base_error : public std::logic_error {
	using std::logic_error::logic_error;
	void write_to(writer &w) const { w.write(what()); }
};
struct basic_error : public base_error {
	using base_error::base_error;
};
struct fatal_error : public base_error {
	using base_error::base_error;
};
struct warning_error : public base_error {
	using base_error::base_error;
};


class error_manager {
public:
	typedef std::function<bool (std::exception &)> error_handler;

private:
	std::unordered_map<std::type_index, error_handler> _map;
	file_writer _log;
	spin_lock _lock;

	error_manager(FILE *f): _log(f) {}

	template <typename E>
	void do_raise(E &&e, warning_error &) {
		_log("Warning: %s\n", e.what());
	}

	template <typename E>
	void do_raise(E &&e, fatal_error &) {
		_log("Fatal error: %s\n", e.what());
		exit(-1);
	}

	template <typename E>
	void do_raise(E &&e, std::exception &) { throw std::forward<E>(e); }

	seal_macro_non_copy(error_manager)

public:
	static error_manager &get() {
		static error_manager emgr(stderr);
		return emgr;
	}

	file_writer &default_logger() { return _log; }

	template <typename E>
	error_handler set_error_handler(error_handler h) {
		std::type_index k = typeid(E);
		std::lock_guard<spin_lock> g(_lock);
		auto i = _map.find(k);
		if ( i == _map.end() ) {
			if ( h ) _map.emplace(k, std::move(h));
			return error_handler();
		} else {
			std::swap(i->second, h);
			if ( !i->second ) _map.erase(i);
			return std::move(h);
		}
	}

	template <typename E, typename F, typename is_return<void, F (E &)>::enable = 0>
	error_handler set_error_handler(F h) {
		return set_error_handler<E>(error_handler([h] (std::exception &e) { h(static_cast<E &>(e)); return false; }));
	}

	template <typename E, typename F, typename is_return<bool, F (E &)>::enable = 0>
	error_handler set_error_handler(F h) {
		return set_error_handler<E>(error_handler([h] (std::exception &e) { return h(static_cast<E &>(e)); }));
	}

	template <typename E>
	void clean_error_handler() {
		std::lock_guard<spin_lock> g(_lock);
		_map.erase(typeid(E));
	}

	file_writer &set_default_logger(FILE *f) {
		return _log.set_file(f);
	}

	template <typename E>
	bool handle_error(E &e) {
		error_handler h;
		{
			std::lock_guard<spin_lock> g(_lock);
			auto f = _map.find(typeid(E));
			if ( f == _map.end() ) return false;
			h = f->second;
		}
		return h(e);
	}

	template <typename E>
	void raise(E &&e) {
		if ( handle_error(e) ) return;
		do_raise(std::forward<E>(e), e);
	}
};

template <typename E>
inline void raise(E &&e) { error_manager::get().raise(std::forward<E>(e)); }


struct file_error : public basic_error {
	using basic_error::basic_error;
};

}

#endif // __SEAL_ERROR_H__

