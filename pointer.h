
#ifndef __SEAL_POINTER_H__
#define __SEAL_POINTER_H__

#include <memory>


namespace sea {

template <typename, typename> struct ptr_maker;

/// 偏特化std::shared_ptr
/// 封装make_shared方法, 返回一个shared_ptr<T>指针
/// 使用I的目的是因为ptr_maker可以make shared_ptr和unique_ptr, 需要用到偏特化的特性达到make
/// shred还是unique的目的
template <typename I, typename T>
struct ptr_maker<std::shared_ptr<I>, T> {
	typedef std::shared_ptr<T> type;

	/// 返回一个 shared_ptr<T>, 以args为T的构造参数
	template <typename ... Args>
	type make(Args &&... args) const {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
};
/// 偏特化std::unique_ptr
template <typename I, typename T>
struct ptr_maker<std::unique_ptr<I>, T> {
	typedef std::unique_ptr<T> type;

	/// 返回unique_ptr<T>类型, 以args为T的构造参数
	template <typename ... Args>
	type make(Args &&... args) const {
		return type(new T (std::forward<Args>(args)...));
	}
};

/** 
 * 封装实现了接口I的类T, 用construct方法new出一个类T, 然后用其数据成员_ptr(P类型)类掌管这个资源;
 * 提供了get方法获取资源, 并提供重载的强制类型转换I类型, 可以隐式的转换成其资源对象.
 * 
 * I 是接口类型
 * 类中的数据成员 _ptr 掌管一个I类资源, 实际资源可能是I的子类对象
 */
template <typename I, typename P = std::unique_ptr<I>>
class abstract_wrapper {
public:
	typedef I interface_type;
	typedef P ptr_type;

	/// 以args为参数构造一个对象T, 并返回该对象;
	/// 该对象作为一个new出来的资源交给类成员_ptr掌管, _ptr为P类型
	/// T 是I的实现类或者说子类
	template <typename T, typename ... Args>
	T &construct(Args &&... args) {
		/// unique_ptr<T>, 以args为构造参数构造T
		auto p = ptr_maker<ptr_type, T>().make(std::forward<Args>(args)...);
		T &o = *p;
		_ptr = std::move(p);	/// 将p的资源移交给_ptr成员掌管
		return o;
	}

	/// 返回_ptr所掌管的资源, 一个是非const&, 一个是const&
	interface_type &get() { return *_ptr; }
	typename std::add_const<interface_type>::type &get() const { return *_ptr; }

	/// 定义强制类型转换类型 interface_type&, 转换为_ptr所掌管的资源
	operator interface_type &() { return get(); }
	operator typename std::add_const<interface_type>::type &() const { return get(); }

private:
	ptr_type _ptr;	/// 掌管一个I类资源, 实际资源可能是I的子类对象
};

}

#endif // __SEAL_POINTER_H__
