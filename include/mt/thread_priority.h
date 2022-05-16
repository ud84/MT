/**
 * ThreadPriority.h : Defines the helper change thread priority
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <thread>

#ifndef _WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif

namespace MT
{

enum PriorityType
{
	ptLow,
	ptNormal,
	ptHigher,
	ptRealTime
};

static int set_thread_priority(std::thread &th, PriorityType tp)
{
#ifndef _WIN32
	if (tp >= ptHigher)
	{
		int policy = 0;
		struct sched_param param = { 0 };
		pthread_getschedparam(th.native_handle(), &policy, &param);
		param.sched_priority = sched_get_priority_max(policy);
		return pthread_setschedparam(th.native_handle(), policy, &param);
	}
#else
	int pm = THREAD_PRIORITY_NORMAL;
	switch (tp)
	{
		case ptLow:
			pm = THREAD_PRIORITY_LOWEST;
		break;
		case ptHigher:
			pm = THREAD_PRIORITY_HIGHEST;
		break;
		case ptRealTime:
			pm = THREAD_PRIORITY_TIME_CRITICAL;
		break;
		default:
		break;
	}

	return SetThreadPriority(th.native_handle(), pm);
#endif

	return 0;
}

}
