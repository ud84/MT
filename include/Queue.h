/**
 * Queue.h : Defines the queue, containing received data
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <queue>
#include <mutex>

#include <MT/Semaphore.h>

namespace MT
{

class IQueueCallback
{
public:
	virtual void QueueOverflow() = 0;

protected:
	~IQueueCallback() {}
};

template <typename data_t>
class Queue
{
public:
	Queue(const uint32_t threadsCount_, uint32_t limit_ = 100)
		: queue(),
		mutex(),
		semaphore(),
		threadsCount(threadsCount_),
		limit(limit_),
		queueCallback(nullptr) {}
	
	void push(const data_t &data)
	{
		{
			std::lock_guard<std::mutex> lock(mutex);
			queue.push(data);
			if (queue.size() > limit)
			{
				while (queue.size() != 0)
				{
					queue.pop();
				}

				if (queueCallback)
				{
					queueCallback->QueueOverflow();
				}
			}
		}
		semaphore.notify();
	}

	data_t pop()
	{
		std::lock_guard<std::mutex> lock(mutex);
		data_t data;
		if (queue.size() != 0)
		{
			data = queue.front();
			queue.pop();
		}
		return data;
	}

	void wait()
	{
		semaphore.wait();
	}

	void releaseAll()
	{
		std::lock_guard<std::mutex> lock(mutex);

		for (size_t i = 0; i != threadsCount; ++i)
		{
			semaphore.notify();
		}

		while (queue.size() != 0)
		{
			queue.pop();
		}
	}

	void setQueueCallback(IQueueCallback *queueCallback_)
	{
		queueCallback = queueCallback_;
	}

	Queue(const Queue&) = delete;
	Queue& operator=(const Queue&) = delete;
private:
	std::queue<data_t> queue;
	std::mutex mutex;
	MT::Semaphore semaphore;

	uint32_t threadsCount, limit;
	
	IQueueCallback *queueCallback;
};

}
