/**
 * SpinLock.h : Defines the spin lock - very fast syncro primitive (based on busy wait)
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <atomic>

namespace MT
{

class SpinLock
{
public:
	void lock()
	{
		while (lck.test_and_set(std::memory_order_acquire))
		{
		}
	}

	void unlock()
	{
		lck.clear(std::memory_order_release);
	}

private:
	std::atomic_flag lck = ATOMIC_FLAG_INIT;
};

}
