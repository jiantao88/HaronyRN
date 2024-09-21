#ifndef UV_TIMER_H
#define UV_TIMER_H
#include <uv.h>
#include <functional>
namespace uv
{
	class EventLoop;
	class Timer
	{
	public:
		using TimerCallback = std::function<void(Timer*)>;
		using TimerClosedCallback = std::function<void(Timer*)>;

		Timer(EventLoop* loop, uint64_t timeout_, uint64_t repeat_, TimerCallback callback);
		virtual ~Timer();

		void start();
		void stop();
		void close(TimerClosedCallback callback);
		void setRepeat(uint64_t times);

	private:
		bool bStarted;
		uv_timer_t  uvHandle;
		uint64_t    timeout;
		uint64_t    repeat;
		TimerCallback cbOnTimer;

		EventLoop* _loop;

		TimerClosedCallback cbClosed;

	private:
		void onTimer();
		void onClosed();
		// used in by uv, then call onTimer()
		static void uvOnTimer(uv_timer_t* handle);
	};
}

#endif
