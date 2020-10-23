#ifndef TIMING
#define TIMING

/*
#define CLOCK_MONOTONIC 0
#define CLOCK_REALTIME 1
*/

#include <sys/types.h>
#include <time.h>

struct timer;

void *create_timer(struct timer **);
int start_timer(struct timer *);
int stop_timer(struct timer *);
double get_elapsed_time(struct timer *);
const char *get_finishing_time(void);

#endif
