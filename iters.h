
#ifndef __SEAL_RANGE_H__
#define __SEAL_RANGE_H__

#include <iterator>
#include <tuple>
#include <type_traits>


namespace sea {

template <typename __Iter>
class iter_pair {
public:
	typedef __Iter iterator;
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
	value_type operator-(type i) const { return _v - i.v; }
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

}

#endif

