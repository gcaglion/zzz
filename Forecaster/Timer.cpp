#include "timer.h"

sTimer::sTimer() {	//-- timer init

	// get ticks per second
	QueryPerformanceFrequency(&timer_frequency);
	//--
}

void sTimer::start() {	
	QueryPerformanceCounter(&timer_start);
	startTime = timer_start.QuadPart * 1000.0/timer_frequency.QuadPart;
}
char* sTimer::stop() {

	//-- stop timer, compute the elapsed time
	QueryPerformanceCounter(&timer_end);
	elapsedTime = (timer_end.QuadPart-timer_start.QuadPart) * 1000.0/timer_frequency.QuadPart;

	//-- milliseconds to timestring (HH:MI:SS.ms)
	int ms = (int)elapsedTime%1000;
	int ss = (int)(floor(elapsedTime/1000))%60;
	int mi = (int)(floor(elapsedTime/1000/60))%60;
	int hh = (int)(floor(elapsedTime/1000/60/60))%60;
	sprintf_s(elapsedTimeS, 30, "%02d:%02d:%02d.%d", hh, mi, ss, ms);

	return elapsedTimeS;
}
