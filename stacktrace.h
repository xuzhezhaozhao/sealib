
#ifndef __SEAL_STACKTRACE_H__
#define __SEAL_STACKTRACE_H__

#include "writer.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>


namespace sea {

namespace stack_impl {

#include <bfd.h>
#include <dlfcn.h>
#include <execinfo.h>

struct stack_trace_entry {
	std::string file;
	std::string func;
	int line;
};

class stack_trace : public std::vector<stack_trace_entry> {
public:
	void append(const char *f, const char *s, int l) {
		auto n = [](const char *s) { return s ? s : "?"; };
		push_back(stack_trace_entry{n(f), n(s), l});
	}

	void write_to(writer &w) const {
		for (auto &e : *this) {
			w("%s:%d %s\n", e.file.c_str(), e.line, e.func.c_str());
		}
	}
};

class generator {
private:
	bfd *_abfd = nullptr;
	asymbol **_syms;
	bfd_vma _pc;
	const char *_fname;
	const char *_sname;
	unsigned int _line;
	bool _found;
	bool _ready = false;
	stack_trace &_result;

public:
	generator(stack_trace &r): _result(r) {}

	void generate(void *addrs[], int size) {
		for (int i = 0; i < size; ++i) {
			Dl_info dli;
			dladdr(addrs[i], &dli);
			open(dli.dli_fname);
			if ( _ready ) {
				_found = false;
				_pc = (bfd_vma)addrs[i];
				bfd_map_over_sections(_abfd, find_wapper, this);
				parse();
			}
		}
		if ( _abfd != nullptr ) {
			close();
		}
	}

private:
	void open(const char *mname) {
		if ( _abfd != nullptr ) {
			if ( strcmp(mname, _abfd->filename) == 0 ) {
				return;
			}
			close();
		}

		_abfd = bfd_openr(mname, nullptr);
		if ( _abfd == nullptr ) {
			return;
		}
		_abfd->flags |= BFD_DECOMPRESS;

		char **m;
		bool r = bfd_check_format(_abfd, bfd_archive);
		r = !r && bfd_check_format_matches(_abfd, bfd_object, &m);
		if ( !r || (bfd_get_file_flags(_abfd) & HAS_SYMS) == 0 ) {
			return;
		}

		bool dynamic = false;
		long storage = bfd_get_symtab_upper_bound(_abfd);
		if ( storage == 0 ) {
			storage = bfd_get_dynamic_symtab_upper_bound(_abfd);
			dynamic = true;
		}
		if ( storage < 0 ) {
			return;
		}

		_syms = (asymbol **)malloc(storage);
		if ( dynamic ) {
			bfd_canonicalize_dynamic_symtab(_abfd, _syms);
		} else {
			bfd_canonicalize_symtab(_abfd, _syms);
		}
		_ready = true;
	}

	void close() {
		bfd_close(_abfd);
		free(_syms);
		_abfd = nullptr;
		_syms = nullptr;
		_fname = nullptr;
		_sname = nullptr;
		_found = false;
		_ready = false;
	}

	static void find_wapper(bfd *, asection *sec, void *arg) {
		reinterpret_cast<generator *>(arg)->find(sec);
	}

	void find(asection *section) {
		if ( _found ) {
			return;
		} else if ( (bfd_get_section_flags(_abfd, section) & SEC_ALLOC) == 0 ) {
			return;
		}
		bfd_vma vma = bfd_get_section_vma(_abfd, section);
		if ( _pc < vma ) {
			return;
		}
		bfd_size_type size  = bfd_get_section_size(section);
		if ( _pc >= vma + size) {
			return;
		}
		_found = bfd_find_nearest_line(
				_abfd, section, _syms, _pc - vma,
				&_fname, &_sname, &_line);
	}

	void parse() {
		if ( !_found ) {
			char **s = backtrace_symbols((void **)&_pc, 1);
			_result.append(nullptr, s[0], 0);
			free(s);
		}
		while ( _found ) {
			char *s = bfd_demangle(_abfd, _sname, 3);
			_result.append(_fname, s ? s : _sname, _line);
			free(s);
			_found = bfd_find_inliner_info(_abfd, &_fname, &_sname, &_line);
		}
	}
};

static stack_trace get_stack_trace() {
	static int init = [] () { bfd_init(); return 0; } ();
	++init, --init;
	void *a[1024];
	int n = backtrace(a, 1024);
	stack_trace r;
	generator g = {r};
	g.generate(a, n);
	return std::move(r);
}

}

using stack_impl::get_stack_trace;
using stack_impl::stack_trace;
using stack_impl::stack_trace_entry;

}

#endif

