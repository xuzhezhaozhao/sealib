
#ifndef __SEAL_IO_WRITER_H__
#define __SEAL_IO_WRITER_H__

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ostream>
#include <string>
#include <iterator>


namespace sea {

class writer;

template <typename T> static std::true_type
has_write_to_helper_impl(decltype((void (T::*)(writer &) const)&T::write_to));

template <typename T> static std::false_type
has_write_to_helper_impl(...);

template <typename T> using has_write_to_impl
		= decltype(has_write_to_helper_impl<T>(nullptr));

template <typename T> struct has_write_to : public has_write_to_impl<T> {};


class writer {
public:
	writer() = default;
	virtual ~writer() = default;

	writer &write(char c) { return write(&c, 1); }
	writer &write(const char *s) { return write(s, strlen(s)); }
	writer &write(const std::string &s) { return write(s.data(), s.size()); }

	template <typename T, typename = typename std::enable_if<has_write_to<T>::value, void>::type>
	writer &write(const T &o) {
		o.write_to(*this);
		return *this;
	}

	void nl() { write('\n'); }
	void endl() { nl(); flush(); }

	template <typename T, typename = typename std::enable_if<has_write_to<T>::value, void>::type>
	writer &operator()(const T &o) { return write(o); }

	writer &operator()(const char *f, ...)
		__attribute__ ((format(printf, 2, 3))) {
		va_list p;
		va_start(p, f);
		vformat(f, p);
		va_end(p);
		return *this;
	}
	writer &format(const char *f, ...)
		__attribute__ ((format(printf, 2, 3))) {
		va_list p;
		va_start(p, f);
		vformat(f, p);
		va_end(p);
		return *this;
	}

	virtual writer &write(const void *, size_t) = 0;
	virtual writer &vformat(const char *, va_list) = 0;
	virtual writer &flush() = 0;

	writer(const writer &) = delete;
	writer &operator=(const writer &) = delete;
};


class file_writer : public writer {
private:
	FILE *_file;
public:
	file_writer(FILE *f): _file(f) {}
	~file_writer() noexcept { file_writer::flush(); }
	using writer::write;
	writer &write(const void *p, size_t n) override {
		fwrite(p, 1, n, _file);
		return *this;
	}
	writer &vformat(const char *f, va_list p) override {
		vfprintf(_file, f, p);
		return *this;
	}
	writer &flush() override {
		fflush(_file);
		return *this;
	}
};


class stream_writer : public writer {
private:
	std::ostream &_os;
public:
	stream_writer(std::ostream &s): _os(s) {}
	~stream_writer() noexcept { stream_writer::flush(); }
	using writer::write;
	writer &write(const void *p, size_t n) override {
		_os.write((const char *)p, n);
		return *this;
	}
	writer &vformat(const char *f, va_list p) override {
		char buf[4096];
		int n = vsnprintf(buf, 4096, f, p);
		if ( n > 0 ) {
			stream_writer::write(buf, (size_t)n);
		}
		return *this;
	}
	writer &flush() override {
		_os.flush();
		return *this;
	}
};


class string_writer : public writer {
private:
	std::string &_s;
public:
	string_writer(std::string &s): _s(s) {}
	~string_writer() noexcept { string_writer::flush(); }
	using writer::write;
	writer &write(const void *p, size_t n) override {
		_s.append((const char *)p, n);
		return *this;
	}
	writer &vformat(const char *f, va_list p) override {
		char buf[4096];
		int n = vsnprintf(buf, 4096, f, p);
		if ( n > 0 ) {
			string_writer::write(buf, (size_t)n);
		}
		return *this;
	}
	writer &flush() override { return *this; }
};


class array_writer : public writer {
private:
	char *_array;
	size_t _pos;
public:
	array_writer(char *a): _array(a) {}
	~array_writer() noexcept { array_writer::flush(); }
	using writer::write;
	writer &write(const void *p, size_t n) override {
		memcpy(_array + _pos, p, n);
		_pos += n;
		return *this;
	}
	writer &vformat(const char *f, va_list p) override {
		int n = vsprintf(_array + _pos, f, p);
		_pos += n > 0 ? (size_t)n : 0;
		return *this;
	}
	writer &flush() override {
		_array[_pos] = '0';
		return *this;
	}
};


template <typename __Insertor>
class insertor_writer : public writer {
private:
	__Insertor _bi;
public:
	insertor_writer(__Insertor bi): _bi(bi) {}
	~insertor_writer() noexcept { insertor_writer::flush(); }
	using writer::write;
	writer &write(const void *p, size_t n) override {
		const char *s = (const char *)p;
		std::copy(s, s + n, _bi);
		return *this;
	}
	writer &vformat(const char *f, va_list p) override {
		char buf[4096];
		int n = vsnprintf(buf, 4096, f, p);
		if ( n > 0 ) {
			insertor_writer::write(buf, (size_t)n);
		}
		return *this;
	}
	writer &flush() override { return *this; }
};


class empty_writer : public writer {
public:
	empty_writer() = default;
	~empty_writer() noexcept { empty_writer::flush(); }
	using writer::write;
	writer &write(const void *, size_t) override { return *this; }
	writer &vformat(const char *, va_list) override { return *this; }
	writer &flush() override { return *this; }
};

}

#endif


