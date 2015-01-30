#include "timeutil.h"

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#include "QtCore/QDateTime"

#ifdef _WIN32
unsigned long long get_clock_tick_ts()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceCounter(&li))
		return 0;
	return li.QuadPart;
}

unsigned long long get_clock_tick_ts_diff_in_ms(unsigned long long ct_start, unsigned long long ct_end)
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
		return 0;
	const double freq = double(li.QuadPart) / 1000.0;
	return double(ct_end - ct_start) / freq;
}
#endif

unsigned long long get_local_timestamp()
{
	return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

unsigned long long get_local_timestamp_diff(unsigned long long begin, unsigned long long end)
{
	return (end - begin);
}
