
#ifndef _SEAL_MEMUSE_H_
#define _SEAL_MEMUSE_H_

#include "writer.h"

#include <cstdio>
#include <malloc.h>
#include <string>


namespace sea {

class memuse {
private:
	unsigned int _mem, _max;

public:
	memuse() { update(); }

	void update() {
		static unsigned int max = 0;
		struct mallinfo info = mallinfo();
		unsigned int m = 0;
		m += (unsigned int)info.uordblks;
		m += (unsigned int)info.hblkhd;
		m += (unsigned int)info.fordblks;
		if ( m > max ) {
			max = m;
		}
		_mem = m;
		_max = max;
	}

	unsigned int mem() const { return _mem; }
	unsigned int max() const { return _max; }

	void write_to(writer &w) {
		w("%uMB, peak %uMB", mem() >> 20, max() >> 20);
	}

};

void write_memuse(writer &w) {
	return memuse().write_to(w);
}

}

#endif


