/**
 * RWLock.h : Defines the RWLock primitive
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#ifndef _WIN32
#include <pthread.h>
#else
#include <string>
#include <windows.h>
#include <Common/WindowsVersion.h>
#endif

#include <assert.h>
#include <memory>

namespace MT
{

#ifndef _WIN32
class RWLock
{
public:
	RWLock()
		: rwlock(PTHREAD_RWLOCK_INITIALIZER)
	{
		pthread_rwlock_init(&rwlock, NULL);
	}

	~RWLock()
	{
		pthread_rwlock_destroy(&rwlock);
	}
	
	void readLock()
	{
		pthread_rwlock_rdlock(&rwlock);
	}
	void readUnLock()
	{
		pthread_rwlock_unlock(&rwlock);
	}

	void writeLock()
	{
		pthread_rwlock_wrlock(&rwlock);
	}
	void writeUnLock()
	{
		pthread_rwlock_unlock(&rwlock);
	}

private:
	pthread_rwlock_t rwlock;
};
#else

typedef struct _RTL_RWLOCK
{
	RTL_CRITICAL_SECTION rtlCS;

	HANDLE hSharedReleaseSemaphore;
	UINT   uSharedWaiters;

	HANDLE hExclusiveReleaseSemaphore;
	UINT   uExclusiveWaiters;

	INT    iNumberActive;
	HANDLE hOwningThreadId;
	DWORD  dwTimeoutBoost;
	PVOID  pDebugInfo;
} RTL_RWLOCK, *LPRTL_RWLOCK;

typedef void(__stdcall *RtlManagePtr)(LPRTL_RWLOCK);
typedef BYTE(__stdcall *RtlOperatePtr)(LPRTL_RWLOCK, BYTE);

class RWLockXP // Implementation for Windows XP
{
public:
	RWLockXP()
		: hGetProcIDDLL(NULL),
		RtlDeleteResource_func(NULL),
		RtlReleaseResource_func(NULL),
		RtlAcquireResourceExclusive_func(NULL),
		RtlAcquireResourceShared_func(NULL),
		rtlRWLock()
	{
		wchar_t path[MAX_PATH] = { 0 };
		GetSystemDirectory(path, sizeof(path));
		std::wstring dllPath = std::wstring(path) + L"\\ntdll.dll";
		HINSTANCE hGetProcIDDLL = LoadLibrary(dllPath.c_str());
		if (hGetProcIDDLL)
		{
			RtlDeleteResource_func = (RtlManagePtr)GetProcAddress(hGetProcIDDLL, "RtlDeleteResource");
			if (!RtlDeleteResource_func)
			{
				return;
			}
			RtlReleaseResource_func = (RtlManagePtr)GetProcAddress(hGetProcIDDLL, "RtlReleaseResource");
			if (!RtlReleaseResource_func)
			{
				return;
			}
			RtlAcquireResourceExclusive_func = (RtlOperatePtr)GetProcAddress(hGetProcIDDLL, "RtlAcquireResourceExclusive");
			if (!RtlAcquireResourceExclusive_func)
			{
				return;
			}
			RtlAcquireResourceShared_func = (RtlOperatePtr)GetProcAddress(hGetProcIDDLL, "RtlAcquireResourceShared");
			if (!RtlAcquireResourceShared_func)
			{
				return;
			}

			RtlManagePtr RtlInitializeResource_func = (RtlManagePtr)GetProcAddress(hGetProcIDDLL, "RtlInitializeResource");
			if (RtlInitializeResource_func)
			{
				RtlInitializeResource_func(&rtlRWLock);
			}
		}
	}
	
	~RWLockXP()
	{
		if (RtlDeleteResource_func)
		{
			RtlDeleteResource_func(&rtlRWLock);
		}
		if (hGetProcIDDLL)
		{
			FreeLibrary(hGetProcIDDLL);
		}
	}
	
	void readLock()
	{
		if (RtlAcquireResourceShared_func)
		{
			RtlAcquireResourceShared_func(&rtlRWLock, TRUE);
		}
	}
	void readUnLock()
	{
		if (RtlReleaseResource_func)
		{
			RtlReleaseResource_func(&rtlRWLock);
		}
	}
	
	void writeLock()
    {
		if (RtlAcquireResourceExclusive_func)
		{
			RtlAcquireResourceExclusive_func(&rtlRWLock, TRUE);
		}
	}
	
	void writeUnLock()
	{
		if (RtlReleaseResource_func)
		{
			RtlReleaseResource_func(&rtlRWLock);
		}
	}

private:
	HINSTANCE hGetProcIDDLL;
	RtlManagePtr RtlDeleteResource_func;
	RtlManagePtr RtlReleaseResource_func;
	RtlOperatePtr RtlAcquireResourceExclusive_func;
	RtlOperatePtr RtlAcquireResourceShared_func;

	RTL_RWLOCK rtlRWLock;
};

typedef void(__stdcall *SRWLock_fptr)(PSRWLOCK);

class RWLockSRW // For Windows Vista+ based on Slim RWLock
{
public:
	RWLockSRW()
		: hGetProcIDDLL(NULL), 
		AcquireSRWLockShared_func(NULL),
		ReleaseSRWLockShared_func(NULL),
		AcquireSRWLockExclusive_func(NULL),
		ReleaseSRWLockExclusive_func(NULL),
		srwLock()
	{
		wchar_t path[MAX_PATH] = { 0 };
		GetSystemDirectory(path, sizeof(path));
		std::wstring dllPath = std::wstring(path) + L"\\kernel32.dll";
		HINSTANCE hGetProcIDDLL = LoadLibrary(dllPath.c_str());
		if (hGetProcIDDLL)
		{
			AcquireSRWLockShared_func = (SRWLock_fptr)GetProcAddress(hGetProcIDDLL, "AcquireSRWLockShared");
			if (!AcquireSRWLockShared_func)
			{
				return;
			}
			ReleaseSRWLockShared_func = (SRWLock_fptr)GetProcAddress(hGetProcIDDLL, "ReleaseSRWLockShared");
			if (!ReleaseSRWLockShared_func)
			{
				return;
			}
			AcquireSRWLockExclusive_func = (SRWLock_fptr)GetProcAddress(hGetProcIDDLL, "AcquireSRWLockExclusive");
			if (!AcquireSRWLockExclusive_func)
			{
				return;
			}
			ReleaseSRWLockExclusive_func = (SRWLock_fptr)GetProcAddress(hGetProcIDDLL, "ReleaseSRWLockExclusive");
			if (!ReleaseSRWLockExclusive_func)
			{
				return;
			}

			SRWLock_fptr InitializeSRWLock_func = (SRWLock_fptr)GetProcAddress(hGetProcIDDLL, "InitializeSRWLock");
			if (InitializeSRWLock_func)
			{
				InitializeSRWLock_func(&srwLock);
			}
		}
	}

	~RWLockSRW()
	{
		if (hGetProcIDDLL)
		{
			FreeLibrary(hGetProcIDDLL);
		}
	}
	
	void readLock()
	{
		if (AcquireSRWLockShared_func)
		{
			AcquireSRWLockShared_func(&srwLock);
		}
	}
	void readUnLock()
	{
		if (ReleaseSRWLockShared_func)
		{
			ReleaseSRWLockShared_func(&srwLock);
		}
	}

	void writeLock()
	{
		if (AcquireSRWLockExclusive_func)
		{
			AcquireSRWLockExclusive_func(&srwLock);
		}
	}
	void writeUnLock()
	{
		if (ReleaseSRWLockExclusive_func)
		{
			ReleaseSRWLockExclusive_func(&srwLock);
		}
	}

private:
	HINSTANCE hGetProcIDDLL;

	SRWLock_fptr AcquireSRWLockShared_func;
	SRWLock_fptr ReleaseSRWLockShared_func;
	SRWLock_fptr AcquireSRWLockExclusive_func;
	SRWLock_fptr ReleaseSRWLockExclusive_func;

	RTL_SRWLOCK srwLock;
};

class RWLock // Wrapper
{
public:
	RWLock()
		: isVistaPlus(Common::IsWindowsVistaOrGreater()), rwLockXP(!isVistaPlus ? new RWLockXP() : nullptr), rwLockSRW(isVistaPlus ? new RWLockSRW() : nullptr) {}

	void readLock()
	{
		if (isVistaPlus)
		{
			rwLockSRW->readLock();
		}
		else
		{
			rwLockXP->readLock();
		}
	}
	void readUnLock()
	{
		if (isVistaPlus)
		{
			rwLockSRW->readUnLock();
		}
		else
		{
			rwLockXP->readUnLock();
		}
	}

	void writeLock()
	{
		if (isVistaPlus)
		{
			rwLockSRW->writeLock();
		}
		else
		{
			rwLockXP->writeLock();
		}
	}
	void writeUnLock()
	{
		if (isVistaPlus)
		{
			rwLockSRW->writeUnLock();
		}
		else
		{
			rwLockXP->writeUnLock();
		}
	}

private:
	bool isVistaPlus;
	std::unique_ptr<RWLockXP> rwLockXP;
	std::unique_ptr<RWLockSRW> rwLockSRW;
};
#endif

class ScopedRWLock
{
public:
	ScopedRWLock(RWLock *lc_, bool write_ = false)
		: lc(*lc_), write(write_), locked(false)
	{
		if (write)
		{
			lc.writeLock();
		}
		else
		{
			lc.readLock();
		}
	}

	~ScopedRWLock()
	{
		if (write)
		{
			lc.writeUnLock();
		}
		else
		{
			lc.readUnLock();
		}
	}

	static void *operator new(size_t) = delete;
	static void operator delete(void *) = delete;
	ScopedRWLock(const ScopedRWLock&) = delete;
	void operator=(const ScopedRWLock&) = delete;
private:
	RWLock &lc;
	bool write, locked;
};

}
