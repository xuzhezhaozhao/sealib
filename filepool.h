
#ifndef __SEAL_FILEPOOL_H__
#define __SEAL_FILEPOOL_H__

#include "error.h"
#include "hash.h"
#include "macro.h"
#include "path.h"
#include "threads.h"

#include <string>

#include <cstdio>


namespace sea {

namespace posix {
#include <sys/stat.h>
}


class file_pool {
private:
	static constexpr int FGR = 1;
	static constexpr int FGW = 2;
	static constexpr int FGA = 4;
	static constexpr int FGP = 8;
	static constexpr int FGB = 16;

	struct opened {
		int flag;
		FILE *file;
		int rc;
	};
	sea::hash_map<size_t, opened> _opened;

	spin_lock _lock;

	file_pool() = default;
	~file_pool() noexcept {
		for (auto &p : _opened) {
			fclose(p.second.file);
		}
	}

	seal_macro_non_copy(file_pool)

	static file_pool &instance() {
		static file_pool fp;
		return fp;
	}

	FILE *open_impl(const char *p, const char *m) {
		if ( !p || *p == '\0' || strcmp(p, "null") == 0 ) {
			p = "/dev/null";
		} else if ( strcmp(p, "stdin") == 0 ) {
			return stdin;
		} else if ( strcmp(p, "stdout") == 0 ) {
			return stdout;
		} else if ( strcmp(p, "stderr") == 0 ) {
			return stderr;
		}

		int fg = 0;
		fg |= strchr(m, 'r') ? FGR : 0;
		fg |= strchr(m, 'w') ? FGW : 0;
		fg |= strchr(m, 'a') ? FGA : 0;
		fg |= strchr(m, '+') ? FGP : 0;
		fg |= strchr(m, 'b') ? FGB : 0;

		if ( strcmp(p, "-") == 0 ) {
			if ( (fg & (FGW | FGA | FGP)) == 0 ) {
				return stdin;
			} else if ( (fg & (FGR | FGP)) == 0 ) {
				return stdout;
			}
		}

		using namespace posix;
		struct stat sb;
		int rt = stat(p, &sb);
		std::lock_guard<spin_lock> lg(_lock);
		if ( rt != 0 || _opened.count(sb.st_ino) == 0 ) {
			FILE *f = fopen(p, m);
			if ( f == nullptr ) {
				raise(cannot_open(p, m));
			} else {
				size_t n = inode(f);
				_opened.insert({n, opened{fg, f, 1}});
			}
			return f;
		} else {
			auto i = _opened.find(sb.st_ino);
			if ( i->second.flag == fg ) {
				++i->second.rc;
				return i->second.file;
			}
			raise(cannot_open(p, m));
			return nullptr;
		}
	}

	void close_impl(FILE *f) {
		if ( f == stdin || f == stdout || f == stderr ) {
			return;
		}
		size_t n = inode(f);
		std::lock_guard<spin_lock> lg(_lock);
		auto i = _opened.find(n);
		if ( i != _opened.end() && --i->second.rc == 0 ) {
			fclose(i->second.file);
			_opened.erase(i);
		}
	}

	size_t inode(FILE *f) {
		using namespace posix;
		struct stat sb;
		fstat(fileno(f), &sb);
		return sb.st_ino;
	}

	static file_error cannot_open(const char *p, const char *m) {
		char s[1024];
		snprintf(s, 1024, "cannot open file \"%s\" with flag \"%s\"\n", p, m);
		return file_error(s);
	}

public:
	static FILE *open(const char *p, const char *m) {
		return instance().open_impl(p, m);
	}

	static FILE *open(const std::string &p, const std::string &m) {
		return open(p.data(), m.data());
	}

	static void close(FILE *f) {
		instance().close_impl(f);
	}

};

}

#endif // __SEAL_FILEPOOL_H__

