
#ifndef __SEAL_TIMER_H__
#define __SEAL_TIMER_H__

#include "writer.h"

#include <chrono>


namespace sea {

template <typename __Clock> class basic_timer {
public:
	typedef __Clock clock_type;
	typedef basic_timer<clock_type> type;

	typedef std::chrono::nanoseconds nanosec;
	typedef std::chrono::time_point<clock_type, nanosec> time_point;

private:
	time_point _start;
	nanosec _time;

public:
	void start() {
		_time = nanosec{0};
		resume();
	}
	void resume() { _start = clock_type::now(); }
	void stop() { _time += clock_type::now() - _start; }

	double second() const { return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(_time).count(); }

	void write_to(writer &w) const {
		auto h = std::chrono::duration_cast<std::chrono::hours>(_time);
		auto m = std::chrono::duration_cast<std::chrono::minutes>(_time - h);
		auto s = std::chrono::duration_cast<std::chrono::seconds>(_time - h - m);
		auto a = std::chrono::duration_cast<std::chrono::milliseconds>(_time - h - m - s);
		if ( h.count() != 0 ) {
			w.format("%dhr", (int)h.count());
		}
		if ( m.count() != 0 ) {
			w.format("%dmin", (int)m.count());
		}
		w.format("%d.%02dsec", (int)s.count(), (int)a.count() / 10);
	}

};

typedef basic_timer<std::chrono::system_clock> sys_timer;
typedef basic_timer<std::chrono::steady_clock> timer;
typedef basic_timer<std::chrono::high_resolution_clock> high_timer;

}

#endif

