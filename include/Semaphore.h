/**
 * Semaphore.h : Defines the Semaphore primitive
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <mutex>
#include <chrono>
#include <condition_variable>

namespace MT
{

class Semaphore
{
public:
	Semaphore()
		: mutex(), condVar(), count(0) {}

	void notify()
	{
		std::unique_lock<std::mutex> lock(mutex);
		++count;
		condVar.notify_one();
	}

	void wait()
	{
		std::unique_lock<std::mutex> lock(mutex);
		while (count == 0)
		{
			condVar.wait(lock);
		}
		--count;
	}

	bool waitFor(uint32_t duration) // duration in milliseconds, return true if timeout is out
	{
		std::unique_lock<std::mutex> lock(mutex);
		while (count == 0)
		{
			if (condVar.wait_for(lock, std::chrono::milliseconds(duration)) == std::cv_status::timeout)
			{
				return true;
			}
		}
		--count;

		return false;
	}

private:
	std::mutex mutex;
	std::condition_variable condVar;
	uint32_t count;
};

}
