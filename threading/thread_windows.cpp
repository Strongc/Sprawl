#include "thread.hpp"
#include "../time/time.hpp"

namespace ThreadStatic
{
	static DWORD WINAPI EntryPoint(LPVOID data)
	{
		sprawl::threading::Thread* thread = reinterpret_cast<sprawl::threading::Thread*>(data);
		sprawl::threading::RunThread(thread);
		return 0;
	}
}

int64_t sprawl::threading::Handle::GetUniqueId() const
{
	return GetThreadId(m_thread);
}

void sprawl::threading::Thread::Start()
{
	m_handle.GetNativeHandle() = CreateThread( nullptr, 0, &ThreadStatic::EntryPoint, this, 0, nullptr);
	if(m_handle.GetNativeHandle() != INVALID_HANDLE_VALUE)
	{
		if(m_threadName != nullptr)
		{
			const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
			struct THREADNAME_INFO
			{
				DWORD dwType; // Must be 0x1000.
				LPCSTR szName; // Pointer to name (in user addr space).
				DWORD dwThreadID; // Thread ID (-1=caller thread).
				DWORD dwFlags; // Reserved for future use, must be zero.
			};
#pragma pack(pop)

			THREADNAME_INFO info;
			info.dwType = 0x1000;
			info.szName = m_threadName;
			info.dwThreadID = GetThreadId(m_handle.GetNativeHandle());
			info.dwFlags = 0;

			__try
			{
				RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
			}
		}
	}
}

void sprawl::threading::Thread::PlatformJoin()
{
	WaitForSingleObject(m_handle.GetNativeHandle(), INFINITE);
}

void sprawl::threading::Thread::PlatformDetach()
{
	CloseHandle(m_handle.GetNativeHandle());
}

sprawl::threading::Handle sprawl::this_thread::GetHandle()
{
	return sprawl::threading::Handle(GetCurrentThread());
}

sprawl::threading::Thread::~Thread()
{
	if(Joinable())
	{
		abort();
	}
	CloseHandle(m_handle.GetNativeHandle());
}

void sprawl::this_thread::Sleep(uint64_t nanoseconds)
{
	::Sleep(nanoseconds / sprawl::time::Resolution::Milliseconds);
}

void sprawl::this_thread::SleepUntil(uint64_t nanosecondTimestamp)
{
	Sleep(nanosecondTimestamp - sprawl::time::Now(sprawl::time::Resolution::Nanoseconds));
}

void sprawl::this_thread::Yield()
{
	SwitchToThread();
}