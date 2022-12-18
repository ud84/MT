/**
 * spin_lock.h : Defines the spin lock - very fast syncro primitive (based on busy wait)
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Official repository: https://github.com/ud84/mt
 */

#pragma once

#include <atomic>

namespace mt
{

class spin_lock
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
