
#ifndef __SEAL_RANGE_H__
#define __SEAL_RANGE_H__

#include "functor.h"

#include <iterator>


namespace sea {

/**
 * @brief 包装了一对迭代器, 用 begin(), end(), cbegin(), cend(), 访问.

 * 注意其中第二个构造函数, 可以从不同的iter_pair类型类构造, 即iter_pair<T>的T可以不同.
 * 提供了 reverse()方法来返回对应的一对 reverse_iterator
 * 
 */
template <typename __Iter>
class iter_pair {
public:
	typedef __Iter iterator;
	typedef __Iter const_iterator;
	typedef iter_pair<iterator> type;

	typedef typename std::iterator_traits<iterator>::difference_type difference_type;
	typedef typename std::iterator_traits<iterator>::value_type value_type;
	typedef typename std::iterator_traits<iterator>::pointer pointer;
	typedef typename std::iterator_traits<iterator>::reference reference;
	typedef typename std::iterator_traits<iterator>::iterator_category iterator_category;

private:
	iterator _b, _e;
public:
	iter_pair(iterator b, iterator e): _b(b), _e(e) {}

	template <typename I>
	iter_pair(const iter_pair<I> &p): _b(p.begin()), _e(p.end()) {}

	iterator begin() const { return _b; }
	iterator end() const { return _e; }
	iterator cbegin() const { return _b; }
	iterator cend() const { return _e; }

	template <typename T>
	T as() const { return T{_b, _e}; }

	size_t size() const { return std::distance(_b, _e); }
	bool empty() const { return _b == _e; }
	reference front() const { return *_b; }
	reference back() const { return *(prev(_e)); }

	std::tuple<iterator, iterator> tuple() const { return std::make_tuple(_b, _e); }
	std::pair<iterator, iterator> pair() const { return {_b, _e}; }


	/**
	 * @brief 返回相应的reverse_iterator对
	 *
	 * @return 
	 */
	iter_pair<std::reverse_iterator<iterator>> reverse() const {
		typedef std::reverse_iterator<iterator> ri;
		return {ri(_e), ri(_b)};
	}
};


/**
 * @brief 类似于std::make_pair
 *
 * @tparam __Iter
 * @param b 起始迭代器
 * @param e 末尾迭代器
 *
 * @return b, e构造的iter_pair对象
 */
template <typename __Iter>
iter_pair<__Iter>
ipair(__Iter b, __Iter e) {
	return {b, e};
}

/**
 * @brief 构造迭代器对
 *
 * @tparam __T
 * @param p 起始迭代器
 * @param n 迭代器范围大小
 *
 * @return 迭代器对
 */
template <typename __T>
iter_pair<__T *>
ipair(__T *p, size_t n) {
	return {p, p + n};
}

/**
 * @brief 以容器的begin(), end()迭代器来构造迭代器对
 *
 * @tparam __Container
 * @param c 容器
 *
 * @return 迭代器对
 */
template <typename __Container>
iter_pair<typename __Container::iterator>
ipair(__Container &c) {
	return {c.begin(), c.end()};
}

template <typename __Container>
/**
 * @brief 构造 const iterator 对
 *
 * @param c 容器
 *
 * @return 
 */
iter_pair<typename __Container::const_iterator>
ipair(const __Container &c) {
	return {c.begin(), c.end()};
}

/**
 * @brief 以容器的 cbegin(), cend() 构造const iterater 对
 *
 * @tparam __Container
 * @param c 容器
 *
 * @return const iterater对
 */
template <typename __Container>
iter_pair<typename __Container::const_iterator>
cipair(__Container &c) {
	return {c.cbegin(), c.cend()};
}

template <typename __Iter>
/**
 * @brief 以 std::pair 结构的两个变量来构造迭代器对 
 *
 * @param p std::pair 结构
 *
 * @return 迭代器对
 */
iter_pair<__Iter>
ipair(const std::pair<__Iter, __Iter> &p) {
	return {p.first, p.second};
}


/**
 * @brief 整数迭代器类型
 *
 * 整数迭代器类型, 随机存取类型, 重载了各种运算符, 迭代器之间的运算实际就是其内部
 * 整数类型之间的运算, 迭代器之间的距离就是内部整数之差
 * 
 * @tparam __T
 */
template <typename __T>
class integer_iter : public std::iterator<std::random_access_iterator_tag, __T, __T, __T *, __T> {
public:
	typedef std::iterator<std::random_access_iterator_tag, __T, __T, __T *, __T> base;
	using typename base::value_type;
	typedef integer_iter<value_type> type;

private:
	value_type _v;

public:
	integer_iter() = default;
	integer_iter(value_type v): _v(v) {}

	bool operator==(integer_iter i) const { return _v == i._v; }
	bool operator!=(integer_iter i) const { return _v != i._v; }
	bool operator< (integer_iter i) const { return _v <  i._v; }
	bool operator> (integer_iter i) const { return _v >  i._v; }
	bool operator<=(integer_iter i) const { return _v <= i._v; }
	bool operator>=(integer_iter i) const { return _v >= i._v; }

	type &operator++() { _v++; return *this; }
	type &operator--() { _v--; return *this; }
	type operator++(int) { type i = *this; _v++; return i; }
	type operator--(int) { type i = *this; _v--; return i; }
	type &operator+=(value_type v) { _v += v; return *this; }
	type &operator-=(value_type v) { _v -= v; return *this; }
	type operator+(value_type v) const { return type(_v + v); }
	type operator-(value_type v) const { return type(_v - v); }
	value_type operator-(type i) const { return _v - i._v; }
	value_type operator*() const { return _v; }
	value_type operator[](value_type n) const { return _v + n; }
};

/**
 * @brief 返回一个整数型的迭代器对, 用来表示一个区间
 *
 * @tparam __T
 * @param n 区间为[0, n)
 *
 * @return 
 */
template <typename __T>
iter_pair<integer_iter<__T>> range(__T n) {
	typedef integer_iter<__T> it;
	return ipair(it(0), it(n));
}

/**
 * @brief 指定整数范围来构造迭代器对
 *
 * @tparam __T
 * @param b 起始位置
 * @param e 结束位置
 *
 * @return 
 */
template <typename __T>
iter_pair<integer_iter<__T>> range(__T b, __T e) {
	typedef integer_iter<__T> it;
	return ipair(it(b), it(e));
}


/**
 * @brief 类似于迭代器, 通过函数子 __G 对其参数 *__M 进行操作
 *
 * @tparam __M 迭代器类型
 * @tparam __G 是一个functor, 重载了()操作符, 操作数为 (*iter)
 */
template <typename __M, template <typename> class __G>
class map_part_iter {
public:
	typedef map_part_iter type;
	typedef __M impl_type;
	typedef typename std::iterator_traits<__M>::value_type pair_type;
	typedef __G<pair_type> getter;

	typedef typename getter::type value_type;
	typedef typename std::iterator_traits<__M>::difference_type difference_type;
	typedef typename sea::replace_core<typename std::iterator_traits<__M>::reference, value_type>::type reference;
	typedef decltype(&std::declval<reference>()) pointer;
	typedef typename std::iterator_traits<__M>::iterator_category iterator_category;

private:
	impl_type _i;

public:
	map_part_iter() = default;
	map_part_iter(impl_type i): _i(i) {}

	reference operator*() const { return getter()(*_i); }
	pointer operator->() const { return &getter()(*_i); }

	type &operator++() {
		++_i;
		return *this;
	}
	type operator++(int) {
		type i = *this;
		++_i;
		return i;
	}

	type &operator--() {
		--_i;
		return *this;
	}
	type operator--(int) {
		type i = *this;
		--_i;
		return i;
	}

	bool operator==(const type &i) const { return _i == i._i; }
	bool operator!=(const type &i) const { return _i != i._i; }
};


/**
 * @brief 返回一个 map_part_iter 的迭代器对, 可以用来遍历容器的所有key值
 *
 * @tparam M container 类型
 * @param m
 *
 * @return 
 */
template <typename M>
iter_pair<map_part_iter<typename M::iterator, sea::get_first>>
key_view(M &m) { return ipair(m); }

/**
 * @brief 与上面类似, 不过返回的map_part_iter为const_iterator
 *
 * @tparam M
 * @param m
 *
 * @return 
 */
template <typename M>
iter_pair<map_part_iter<typename M::const_iterator, sea::get_first>>
key_view(const M &m) { return ipair(m); }

/**
 * @brief 与上面类似, 不过返回的map_part_iter为const_iterator
 *
 * @tparam M
 * @param m
 *
 * @return 
 */
template <typename M>
iter_pair<map_part_iter<typename M::const_iterator, sea::get_first>>
key_cview(M &m) { return cipair(m); }

/**
 * @brief 重载 pair<I, I> 类型参数
 *
 * @tparam I 迭代器类型
 * @param p
 *
 * @return 
 */
template <typename I>
iter_pair<map_part_iter<I, sea::get_first>>
key_view(const std::pair<I, I> &p) { return ipair(p); }

/**
 * 下面的函数系与上面的函数系类型, 差别是上面是获取关联型容器的key值, 下面获取value值
 */
template <typename M>
iter_pair<map_part_iter<typename M::iterator, sea::get_second>>
val_view(M &m) { return ipair(m); }

template <typename M>
iter_pair<map_part_iter<typename M::const_iterator, sea::get_second>>
val_view(const M &m) { return ipair(m); }

template <typename M>
iter_pair<map_part_iter<typename M::const_iterator, sea::get_second>>
val_cview(M &m) { return cipair(m); }

template <typename I>
iter_pair<map_part_iter<I, sea::get_second>>
val_view(const std::pair<I, I> &p) { return ipair(p); }

}

#endif

