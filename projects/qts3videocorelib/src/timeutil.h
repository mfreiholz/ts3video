#ifndef TIMEUTIL_HEADER
#define TIMEUTIL_HEADER

unsigned long long get_clock_tick_ts();
unsigned long long get_clock_tick_ts_diff_in_ms(unsigned long long ct_start, unsigned long long ct_end);

unsigned long long get_local_timestamp();
unsigned long long get_local_timestamp_diff(unsigned long long begin, unsigned long long end);

#endif