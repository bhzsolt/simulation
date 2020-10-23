#include <stdio.h>
#include <stdlib.h>
#include <timing.h>

struct timer {
	struct timespec start;
	struct timespec end;
};

void *create_timer(struct timer **timer)
{
	return *timer = calloc(1, sizeof(**timer));
}

int start_timer(struct timer *timer)
{
	return clock_gettime(CLOCK_MONOTONIC, &(timer->start));
}

int stop_timer(struct timer *timer)
{
	return clock_gettime(CLOCK_MONOTONIC, &(timer->end));
}

double get_elapsed_time(struct timer *timer)
{
	return timer->end.tv_sec - timer->start.tv_sec + (timer->end.tv_nsec - timer->start.tv_nsec) * 1e-9;
}

const char *get_finishing_time(void)
{
	time_t endtime;
	struct tm *timeinfo;
	static char buffer[64];

	time(&endtime);
	timeinfo = localtime(&endtime);

	sprintf(buffer, "%4d.%02d.%02d_%02d:%02d:%02d",
			timeinfo->tm_year + 1900,
			timeinfo->tm_mon + 1,
			timeinfo->tm_mday,
			timeinfo->tm_hour,
			timeinfo->tm_min,
			timeinfo->tm_sec);
	return buffer;
}
