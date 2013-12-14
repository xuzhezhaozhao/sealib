
#ifndef __SEAL_IO_WRITER_H__
#define __SEAL_IO_WRITER_H__

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ostream>
#include <string>
#include <iterator>


namespace sea {

class writer {
public:
	writer() = default;

	void write(char c) { write(&c, 1); }
	void write(const char *s) { write(s, strlen(s)); }
	void write(const std::string &s) { write(s.data(), s.size()); }

	void operator()(const char *f, ...)
		__attribute__ ((format(printf, 2, 3))) {
		va_list p;
		va_start(p, f);
		vformat(f, p);
		va_end(p);
	}
	void format(const char *f, ...)
		__attribute__ ((format(printf, 2, 3))) {
		va_list p;
		va_start(p, f);
		vformat(f, p);
		va_end(p);
	}

	virtual void write(const void *, size_t) = 0;
	virtual void vformat(const char *, va_list) = 0;
	virtual void flush() = 0;

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
	void write(const void *p, size_t n) override { fwrite(p, 1, n, _file); }
	void vformat(const char *f, va_list p) override { vfprintf(_file, f, p); }
	void flush() override { fflush(_file); }
};


class stream_writer : public writer {
private:
	std::ostream &_os;
public:
	stream_writer(std::ostream &s): _os(s) {}
	~stream_writer() noexcept { stream_writer::flush(); }
	using writer::write;
	void write(const void *p, size_t n) override { _os.write((const char *)p, n); }
	void vformat(const char *f, va_list p) override {
		char buf[4096];
		int n = vsnprintf(buf, 4096, f, p);
		if ( n > 0 ) {
			stream_writer::write(buf, (size_t)n);
		}
	}
	void flush() override { _os.flush(); }
};


class string_writer : public writer {
private:
	std::string &_s;
public:
	string_writer(std::string &s): _s(s) {}
	~string_writer() noexcept { string_writer::flush(); }
	using writer::write;
	void write(const void *p, size_t n) override { _s.append((const char *)p, n); }
	void vformat(const char *f, va_list p) override {
		char buf[4096];
		int n = vsnprintf(buf, 4096, f, p);
		if ( n > 0 ) {
			string_writer::write(buf, (size_t)n);
		}
	}
	void flush() override {}
};


class array_writer : public writer {
private:
	char *_array;
	size_t _pos;
public:
	array_writer(char *a): _array(a) {}
	~array_writer() noexcept { array_writer::flush(); }
	using writer::write;
	void write(const void *p, size_t n) override {
		memcpy(_array + _pos, p, n);
		_pos += n;
	}
	void vformat(const char *f, va_list p) override {
		int n = vsprintf(_array + _pos, f, p);
		_pos += n > 0 ? (size_t)n : 0;
	}
	void flush() override { _array[_pos] = '0'; }
};


template <typename __Insertor>
class insertor_writer : public writer {
private:
	__Insertor _bi;
public:
	insertor_writer(__Insertor bi): _bi(bi) {}
	~insertor_writer() noexcept { insertor_writer::flush(); }
	using writer::write;
	void write(const void *p, size_t n) override {
		const char *s = (const char *)p;
		std::copy(s, s + n, _bi);
	}
	void vformat(const char *f, va_list p) override {
		char buf[4096];
		int n = vsnprintf(buf, 4096, f, p);
		if ( n > 0 ) {
			insertor_writer::write(buf, (size_t)n);
		}
	}
	void flush() override {}
};


class empty_writer : public writer {
public:
	empty_writer() = default;
	~empty_writer() noexcept { empty_writer::flush(); }
	using writer::write;
	void write(const void *, size_t) override {}
	void vformat(const char *, va_list) override {}
	void flush() override {}
};


writer &make_writer(FILE *f) { return *new file_writer(f); }
writer &make_writer(std::ostream &o) { return *new stream_writer(o); }
writer &make_writer(std::string &s) { return *new string_writer(s); }
writer &make_writer(char *a) { return *new array_writer(a); }
template <typename __I, typename = typename std::enable_if<std::is_base_of<std::output_iterator_tag, typename std::iterator_traits<__I>::iterator_category>::value, void>::type>
writer &make_writer(__I i) { return *new insertor_writer<__I>(i); }
writer &make_writer() { return *new empty_writer(); }

}

#endif


