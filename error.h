
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
/// 下面的类型只是作为重载函数的时候区分类型之用
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
	/// 异常处理函数, 返回值为bool, 参数为std::exception
	typedef std::function<bool (std::exception &)> error_handler;

private:
	/// 按异常的种类进行特定处理
	/// 对其操作需要保证原子性
	std::unordered_map<std::type_index, error_handler> _map;
	file_writer _log; 
	spin_lock _lock;	/// 进程锁, 主要用来保证对_map的原子性操作

	error_manager(FILE *f): _log(f) {}

	/// 若_map没有处理此异常的函数, 则由此函数处理
	/// warning error
	template <typename E>
	void do_raise(E &&e, warning_error &) {
		_log("Warning: %s\n", e.what());
	}
	/// fatal error, 并调用exit(-1)终止程序
	template <typename E>
	void do_raise(E &&e, fatal_error &) {
		_log("Fatal error: %s\n", e.what());
		exit(-1);
	}

	/// 处理不属于上面类型的其他异常, 直接throw该异常交给上层处理
	template <typename E>
	void do_raise(E &&e, std::exception &) { throw std::forward<E>(e); }

	seal_macro_non_copy(error_manager)

public:
	/// 单例模式
	static error_manager &get() {
		static error_manager emgr(stderr);
		return emgr;
	}

	file_writer &default_logger() { return _log; }

	/// 设置类型E的error_handler函数, 将其添加到_map中, 返回该函数, 
	/// 若原来有对应的处理函数, 则替换, 返回之前的处理函数
	/// 若h不是可调用的, 那么没有影响
	/// 整个函数是原子操作, 由lock_guard保护
	template <typename E>
	error_handler set_error_handler(error_handler h) {
		std::type_index k = typeid(E);
		std::lock_guard<spin_lock> g(_lock);	/// 构造的时候上锁, 销毁的时候解锁
												/// 确保遇到异常时解锁
		auto i = _map.find(k);
		if ( i == _map.end() ) {
			/// function<> 模板重载了 operate bool, 检测h是否是可调用的
			if ( h ) _map.emplace(k, std::move(h));
			return error_handler();
		} else {
			std::swap(i->second, h);
			if ( !i->second ) _map.erase(i);	/// 不可调用就擦除
			return std::move(h);
		}
	}

	/// 待设置的函数返回值为void, 用lambada包装该函数形成error_handler, 返回false
	template <typename E, typename F, typename is_return<void, F (E &)>::enable = 0>
	error_handler set_error_handler(F h) {
		return set_error_handler<E>(error_handler([h] (std::exception &e) { h(static_cast<E &>(e)); return false; }));
	}

	/// 待设置函数返回值为bool, 用lambada包装该函数形成error_handler, 返回该函数返回值
	template <typename E, typename F, typename is_return<bool, F (E &)>::enable = 0>
	error_handler set_error_handler(F h) {
		return set_error_handler<E>(error_handler([h] (std::exception &e) { return h(static_cast<E &>(e)); }));
	}

	/// 擦除类型E的error_handler, 整个函数是原子操作
	template <typename E>
	void clean_error_handler() {
		std::lock_guard<spin_lock> g(_lock);	/// 确保该函数的原子性
		_map.erase(typeid(E));
	}

	/// 设置_log的文件指针
	file_writer &set_default_logger(FILE *f) {
		return _log.set_file(f);
	}

	/// 查看_map是否定义了处理这种类型error的error_handler, 有就由它处理, 并返回函数返回值, 没有就返回false
	/// 返回false就将该error交给do_raise处理
	template <typename E>
	bool handle_error(E &e) {
		error_handler h;
		{
			/// 花括号里的这一部分是原子操作
			std::lock_guard<spin_lock> g(_lock);
			auto f = _map.find(typeid(E));	/// typeid(E)获取E类型信息
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

/// 将异常交给单例error_manager处理
template <typename E>
inline void raise(E &&e) { error_manager::get().raise(std::forward<E>(e)); }	// 完美转发


/// 新加一个file_error, 与warning_err, fatal_error类似
struct file_error : public basic_error {
	using basic_error::basic_error;
};

}

#endif // __SEAL_ERROR_H__

