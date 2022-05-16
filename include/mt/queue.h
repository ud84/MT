/**
 * Queue.h : Defines the queue, containing received data
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com, 2022
 */

#pragma once

#include <queue>
#include <mutex>
#include <function>

#include <mt/semaphore.h>

namespace mt
{

template <typename data_t>
class queue
{
public:
    queue(const uint32_t threads_count_, uint32_t limit_ = 100, std::function<void(void)> overflow_callback_ = []() {})
        : queue(),
        mutex(),
        semaphore(),
        threads_count(threads_count_),
        limit(limit_),
        overflow_callback(overflow_callback_) {}

    void push(const data_t &data)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue_.push(data);
            if (queue_.size() > limit)
            {
                while (queue_.size() != 0)
                {
                    queue_.pop();
                }

                if (overflow_callback)
                {
                    overflow_callback();
                }
            }
        }
        semaphore.notify();
    }

    data_t pop()
    {
        std::lock_guard<std::mutex> lock(mutex);
        data_t data;
        if (queue_.size() != 0)
        {
            data = queue_.front();
            queue_.pop();
        }
        return data;
    }

    void wait()
    {
        semaphore.wait();
    }

    void release_all()
    {
        std::lock_guard<std::mutex> lock(mutex);

        for (size_t i = 0; i != threads_count; ++i)
        {
            semaphore.notify();
        }

        while (queue_.size() != 0)
        {
            queue_.pop();
        }
    }

    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;

private:
    std::queue<data_t> queue_;
    std::mutex mutex;
    semaphore semaphore;

    uint32_t threads_count, limit;

    std::function<void(void)> overflow_callback;
};

}
