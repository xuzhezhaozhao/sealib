
#ifndef __SEAL_ERROR_H__
#define __SEAL_ERROR_H__

#include "writer.h"

#include <exception>


namespace sea {

struct base_error {
	std::string str;
	base_error(const std::string &s): str(s) {}
	void write_to(writer &w) const { w(str); }
};

class exception_error : public std::exception, public base_error {
public:
	exception_error(const std::string &s): base_error(s) {}
	const char* what() const noexcept override { return str.data(); }
};

class terminate_error : public base_error {
public:
	int error_no;
	terminate_error(const std::string &s, int e = -1): base_error(s), error_no(e) {}
};

class logged_error : public base_error {
public:
	logged_error(const std::string &s): base_error(s) {}
};


void log_err(const std::string &s) {
	fputs(s.data(), stderr);
}

void error(const exception_error &e) { throw e; }

void error(const terminate_error &e) {
	log_err(e.str);
	exit(e.error_no);
}

void error(const logged_error &e) { log_err(e.str); }

}

#endif // __SEAL_ERROR_H__

