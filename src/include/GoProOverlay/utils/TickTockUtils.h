#pragma once

#include <time.h>

#define TICK() \
	timespec __ticktock_start,__ticktock_stop; \
	clock_gettime(CLOCK_MONOTONIC,&__ticktock_start);

#define TOCK() \
	clock_gettime(CLOCK_MONOTONIC,&__ticktock_stop); \
	double __ticktock_delta_sec = ((double)(__ticktock_stop.tv_sec) + __ticktock_stop.tv_nsec / 1.0e9) - \
		((double)(__ticktock_start.tv_sec) + __ticktock_start.tv_nsec / 1.0e9);

#define TOCK_AND_PRINT(NAME) TOCK()	printf("%s: %0.6fs\n", NAME, __ticktock_delta_sec);