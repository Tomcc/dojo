#include "stdafx.h"

#ifdef PLATFORM_WIN32
#include <Windows.h>
#endif

#include "Timer.h"

using namespace Dojo;

double Timer::currentTime()
{
	double d;
#ifdef PLATFORM_WIN32
	__int64 freq, gTime;
	QueryPerformanceCounter((LARGE_INTEGER *)&gTime);  // Get current count
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq); // Get processor freq
	d = (double)(gTime)/(double)freq;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL );
	d = (double)tv.tv_usec/1000000 + (double)tv.tv_sec;
#endif
	return d;
}