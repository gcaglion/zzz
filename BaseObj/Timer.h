#pragma once
#include "../common.h"
#include "time.h"

#define TIMER_TIME_FORMAT_LEN 64
#define TIMER_ELAPSED_FORMAT_LEN 32

struct sTimer {

	double startTime;
	char startTimeS[TIMER_TIME_FORMAT_LEN];
	double stopTime;
	char stopTimeS[TIMER_TIME_FORMAT_LEN];
	double elapsedTime;
	char elapsedTimeS[TIMER_ELAPSED_FORMAT_LEN];

	EXPORT sTimer();

	EXPORT void start();
	EXPORT void stop(char* oElapsedS);

private:
	LARGE_INTEGER timer_frequency;			// ticks per second
	LARGE_INTEGER timer_start;
	LARGE_INTEGER timer_stop;

};

//-- static versions
EXPORT void SgetTimestamp(double time_, char* oTimeS_);
EXPORT void SgetElapsed(double time_, char* oTimeS_);
