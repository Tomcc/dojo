#include "Timer.h"

#ifdef PLATFORM_WIN32
#include "dojo_win_header.h"
#endif

double Dojo::Timer::currentTime() {
	double d;
#ifdef PLATFORM_WIN32
	__int64 freq, gTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&gTime);  // Get current count
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);  // Get processor freq
	d = (double)(gTime) / (double)freq;
#else
	struct timeval tv;
	gettimeofday(&tv, nullptr );
	d = (double)tv.tv_usec / 1000000 + (double)tv.tv_sec;
#endif
	return d;
}
