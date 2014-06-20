
#ifndef __SEAL_POINTER_H__
#define __SEAL_POINTER_H__

#include <memory>


namespace sea {

template <typename, typename> struct ptr_maker;

template <typename I, typename T>
struct ptr_maker<std::shared_ptr<I>, T> {
	typedef std::shared_ptr<T> type;

	template <typename ... Args>
	type make(Args &&... args) const {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
};

template <typename I, typename T>
struct ptr_maker<std::unique_ptr<I>, T> {
	typedef std::unique_ptr<T> type;

	template <typename ... Args>
	type make(Args &&... args) const {
		return type(new T (std::forward<Args>(args)...));
	}
};


template <typename I, typename P = std::unique_ptr<I>>
class abstract_wrapper {
public:
	typedef I interface_type;
	typedef P ptr_type;

	template <typename T, typename ... Args>
	T &construct(Args &&... args) {
		auto p = ptr_maker<ptr_type, T>().make(std::forward<Args>(args)...);
		T &o = *p;
		_ptr = std::move(p);
		return o;
	}

	interface_type &get() { return *_ptr; }
	typename std::add_const<interface_type>::type &get() const { return *_ptr; }

private:
	ptr_type _ptr;
};

}

#endif // __SEAL_POINTER_H__
