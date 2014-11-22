
#ifndef _SEAL_THREADS_H_
#define _SEAL_THREADS_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace sea {


/// 封装了对atomic<>的操作, 进程锁
/// 这个类属于 Lockable[http://www.cplusplus.com/reference/concept/Lockable/] 的范畴
/// mutex类更复杂, 这个更高效
/// 这个类的缺点是多个进程经常抢占一个资源时发生冲突的成本高, 因为lock时是while(true), 并不会block
class spin_lock {
private:
	std::atomic<bool> _a = {false};

public:
	/// 若atomic内的数据是false, 就置为true, 并放回, 
	/// 若为true, 则不断循环, 直到其被其他进程置为false
	void lock() { while ( _a.exchange(true, std::memory_order_acq_rel) ); }
	/// 将atomic内数据置为false
	void unlock() { _a.store(false, std::memory_order_release); }
	/// 只尝试一次锁操作, 返回成功与否, true表示成功, false表示失败
	bool try_lock() { return !_a.exchange(true, std::memory_order_acq_rel); }
	/// 查看是否被锁住了
	bool locked() const { return _a.load(std::memory_order_acquire); }
};


class thread_pool {
private:
	std::vector<std::thread> _threads;
	enum class command {wait, run, stop};
	std::atomic<command> _cmd;
	std::atomic<int> _busy;

	std::mutex _mutex;
	std::condition_variable _cvar;

	const std::function<void (int)> *_func;
	std::atomic<int> _currj;	/// 当前job
	int _totalj;	/// 总jobs数

public:
	thread_pool(int n) { extend_by(n > 0 ? n - 1 : 0); }

	~thread_pool() noexcept { stop(); }

	void extend_by(int n) {
		_busy = n + (int)_threads.size();
		_cmd = command::wait;
		_threads.reserve(n + _threads.size());
		while ( n-- > 0 ) {
			_threads.emplace_back(loop_wrapper, this);
		}
		wait_free();
	}

	void run_njob(int n, const std::function<void (int)> &f) {
		_func = &f;
		_currj = 0;
		_totalj = n;

		// XXX ??
		if ( !_threads.empty() ) {
			_cmd = command::run;
			notify();
		}
		do_run();

		wait_free();
	}

	void run_njob(int n, const std::function<void (int)> &&f) {
		run_njob(n, f);
	}
	void run(const std::function<void (int)> &f) {
		run_njob((int)_threads.size(), f);
	}
	void run(const std::function<void (int)> &&f) {
		run(f);
	}

	void stop() {
		_cmd = command::stop;
		notify();
		for (std::thread &t : _threads) {
			t.join();
		}
		_threads.clear();
		_busy = 0;
		_cmd = command::wait;
	}

	thread_pool(const thread_pool &) = delete;
	thread_pool &operator=(const thread_pool &) = delete;

	static void temporary_run(size_t nth, std::function<void (int)> &f) {
		if ( nth <= 1 ) {
			f(0);
			return;
		}
		std::vector<std::thread> ts;
		ts.reserve(nth);
		int i = 0;
		while ( nth-- > 0 ) {
			ts.emplace_back([&f] (int i) { f(i); }, i++);
		}
		for (std::thread &t : ts) {
			t.join();
		}
	}
	static void temporary_run(size_t nth, std::function<void (int)> &&f) {
		temporary_run(nth, f);
	}

private:
	static void loop_wrapper(thread_pool *p) { p->loop(); }

	void loop() {
		while ( _cmd != command::stop ) {
			if ( _cmd == command::wait ) {
				do_wait();
			} else if ( _cmd == command::run ) {
				do_run();
			}
		}
	}

	void do_wait() {
		if ( --_busy == 0 ) {
			notify();
		}
		std::unique_lock<std::mutex> l{_mutex};
		while ( _cmd == command::wait ) {
			_cvar.wait(l);
		}
		++_busy;
	}

	/// 依次执行_func[0, _totalj)
	void do_run() {
		int j;
		// [0, _totalj)
		while ( (j = _currj++) < _totalj ) {
			(*_func)(j);
		}
		_cmd = command::wait;
	}

	/// block直到_busy为0
	void wait_free() {
		if ( _busy > 0 ) {
			std::unique_lock<std::mutex> l{_mutex};
			while ( _busy > 0 ) {
				_cvar.wait(l);
			}
		}
	}

	// XXX ?? 为啥 先lock(), 再unlock()
	void notify() {
		_mutex.lock();
		_mutex.unlock();
		_cvar.notify_all();
	}
};

}

#endif // _SEAL_THREADS_H_


