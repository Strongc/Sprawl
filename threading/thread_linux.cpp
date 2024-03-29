#include "thread.hpp"
#include "../time/time.hpp"
#include <time.h>

#ifdef __APPLE__
#include <sched.h>
#	define pthread_yield sched_yield
#endif

namespace ThreadStatic
{
	static void* EntryPoint(void *data)
	{
		sprawl::threading::Thread* thread = reinterpret_cast<sprawl::threading::Thread*>(data);
		sprawl::threading::RunThread(thread);
		return nullptr;
	}
}

int64_t sprawl::threading::Handle::GetUniqueId() const
{
	return int64_t(m_thread);
}

void sprawl::threading::Thread::Start()
{
	int result = pthread_create(&m_handle.GetNativeHandle(), nullptr, &ThreadStatic::EntryPoint, this);
#ifndef __APPLE__
	if(result == 0)
	{
		if(m_threadName != nullptr)
		{
			pthread_setname_np(m_handle.GetNativeHandle(), m_threadName);
		}
	}
#endif
}

void sprawl::threading::Thread::PlatformJoin()
{
	void* value;
	pthread_join(m_handle.GetNativeHandle(), &value);
}

void sprawl::threading::Thread::PlatformDetach()
{
	pthread_detach(m_handle.GetNativeHandle());
}

sprawl::threading::Handle sprawl::this_thread::GetHandle()
{
	return sprawl::threading::Handle(pthread_self());
}

sprawl::threading::Thread::~Thread()
{
	if(Joinable())
	{
		abort();
	}
}

void sprawl::this_thread::Sleep(uint64_t nanoseconds)
{
	struct timespec ts;

	ts.tv_sec = time_t(sprawl::time::Convert(nanoseconds, sprawl::time::Resolution::Nanoseconds, sprawl::time::Resolution::Seconds));
	int64_t secsAsNanosecs = sprawl::time::Convert(ts.tv_sec, sprawl::time::Resolution::Seconds, sprawl::time::Resolution::Nanoseconds);
	ts.tv_nsec = long(nanoseconds - secsAsNanosecs);

	nanosleep(&ts, nullptr);
}

void sprawl::this_thread::SleepUntil(uint64_t nanosecondTimestamp)
{
	Sleep(nanosecondTimestamp - sprawl::time::Now(sprawl::time::Resolution::Nanoseconds));
}

void sprawl::this_thread::Yield()
{
	pthread_yield();
}