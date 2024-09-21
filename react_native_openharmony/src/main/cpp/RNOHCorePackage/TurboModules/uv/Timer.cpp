#include "Timer.h"
#include "EventLoop.h"

using namespace uv;

Timer::Timer(EventLoop * loop, uint64_t timeout_, uint64_t repeat_, TimerCallback callback)
    :bStarted(false),
	_loop(loop),
    timeout(timeout_),
    repeat(repeat_),
	cbOnTimer(callback),
    cbClosed(nullptr)
{
    uvHandle.data = static_cast<void*>(this);
    int ret=  uv_timer_init(loop->handle(), &uvHandle);
}

Timer::~Timer()
{
}

void Timer::start()
{
	if (false == bStarted) {
		bStarted = true;
		uv_timer_start(&uvHandle, Timer::uvOnTimer, timeout, repeat);
	}
}
void Timer::stop()
{
	if (uv_is_active((uv_handle_t*)&uvHandle)) {
		uv_timer_stop(&uvHandle);
	}
	bStarted = false;
}

void Timer::uvOnTimer(uv_timer_t * handle)
{
	auto ptr = static_cast<Timer*>(handle->data);
	ptr->onTimer();
}

void Timer::onTimer()
{
	if (cbOnTimer) {
		cbOnTimer(this);
	}
}

void Timer::close(TimerClosedCallback callback)
{
    cbClosed = callback;
	stop();

	if (uv_is_closing((uv_handle_t*)&uvHandle) == 0) {
		uv_close((uv_handle_t*)&uvHandle,
			[](uv_handle_t* handle)
		{
			auto ptr = static_cast<Timer*>(handle->data);
			ptr->onClosed();
		});
	}
	else {
		onClosed();
	}
}

void Timer::onClosed()
{
	if (cbClosed)
		cbClosed(this);
}

void Timer::setRepeat(uint64_t times)
{
    repeat = times;
    ::uv_timer_set_repeat(&uvHandle, times);
}






