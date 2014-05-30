
#ifndef __SEAL_RANGE_H__
#define __SEAL_RANGE_H__

#include "functor.h"

#include <iterator>


namespace sea {

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

	iter_pair<std::reverse_iterator<iterator>> reverse() const {
		typedef std::reverse_iterator<iterator> ri;
		return {ri(_e), ri(_b)};
	}
};


template <typename __Iter>
iter_pair<__Iter>
ipair(__Iter b, __Iter e) {
	return {b, e};
}

template <typename __T>
iter_pair<__T *>
ipair(__T *p, size_t n) {
	return {p, p + n};
}

template <typename __Container>
iter_pair<typename __Container::iterator>
ipair(__Container &c) {
	return {c.begin(), c.end()};
}

template <typename __Container>
iter_pair<typename __Container::const_iterator>
ipair(const __Container &c) {
	return {c.begin(), c.end()};
}

template <typename __Container>
iter_pair<typename __Container::const_iterator>
cipair(__Container &c) {
	return {c.cbegin(), c.cend()};
}

template <typename __Iter>
iter_pair<__Iter>
ipair(const std::pair<__Iter, __Iter> &p) {
	return {p.first, p.second};
}

template <typename __T, typename = typename std::enable_if<std::is_integral<__T>::value, void>::type>
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
	bool operator<(integer_iter i) const { return _v < i._v; }
	bool operator>(integer_iter i) const { return _v > i._v; }
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

template <typename __T, typename = typename std::enable_if<std::is_integral<__T>::value, void>::type>
iter_pair<integer_iter<__T>> range(__T n) {
	typedef integer_iter<__T> it;
	return ipair(it(0), it(n));
}

template <typename __T, typename = typename std::enable_if<std::is_integral<__T>::value, void>::type>
iter_pair<integer_iter<__T>> range(__T b, __T e) {
	typedef integer_iter<__T> it;
	return ipair(it(b), it(e));
}


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


template <typename M>
iter_pair<map_part_iter<typename M::iterator, sea::get_first>>
key_view(M &m) { return ipair(m); }

template <typename M>
iter_pair<map_part_iter<typename M::const_iterator, sea::get_first>>
key_view(const M &m) { return ipair(m); }

template <typename M>
iter_pair<map_part_iter<typename M::const_iterator, sea::get_first>>
key_cview(M &m) { return cipair(m); }

template <typename I>
iter_pair<map_part_iter<I, sea::get_first>>
key_view(const std::pair<I, I> &p) { return ipair(p); }


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

