#pragma once
#include "../common.h"

struct sTimer {

	double startTime;
	double elapsedTime;
	char elapsedTimeS[30];

	sTimer();

	void start();
	char* stop();

private:
	LARGE_INTEGER timer_frequency;			// ticks per second
	LARGE_INTEGER timer_start;
	LARGE_INTEGER timer_end;

};
