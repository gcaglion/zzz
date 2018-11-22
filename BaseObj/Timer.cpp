#include "timer.h"

sTimer::sTimer() {	//-- timer init

	// get ticks per second
	QueryPerformanceFrequency(&timer_frequency);
	//--
}

void getTimestamp(double time_, char* oTimeS_) {
	char* ftime;
	time_t ltime; // calendar time 
	HANDLE TimeMutex;

	TimeMutex = CreateMutex(NULL, TRUE, NULL);
	WaitForSingleObject(TimeMutex, INFINITE);
	ltime = time(NULL); // get current cal time 
	ftime = asctime(localtime(&ltime));
	ftime[strlen(ftime)-1] = '\0';
	ReleaseMutex(TimeMutex);

	strcpy_s(oTimeS_, TIMER_TIME_FORMAT_LEN, ftime);

}

void getElapsed(double time_, char* oTimeS_){	//-- milliseconds to timestring (HH:MI:SS.ms)
	int ms = (int)time_%1000;
	int ss = (int)(floor(time_/1000))%60;
	int mi = (int)(floor(time_/1000/60))%60;
	int hh = (int)(floor(time_/1000/60/60))%60;
	sprintf_s(oTimeS_, 30, "%02d:%02d:%02d.%d", hh, mi, ss, ms);
}

void sTimer::start() {	
	QueryPerformanceCounter(&timer_start);
	startTime = timer_start.QuadPart * 1000.0/timer_frequency.QuadPart;

	getTimestamp(startTime, startTimeS);
}

void sTimer::stop(char* oElapsedS) {

	//-- stop timer, compute the elapsed time
	QueryPerformanceCounter(&timer_stop);
	stopTime = timer_stop.QuadPart * 1000.0/timer_frequency.QuadPart;
	elapsedTime = (timer_stop.QuadPart-timer_start.QuadPart) * 1000.0/timer_frequency.QuadPart;

	getTimestamp(stopTime, stopTimeS);

	//-- build elapsedS
	getElapsed(elapsedTime, elapsedTimeS);

	//-- copy elapsedS to output buffer
	strcpy_s(oElapsedS, TIMER_ELAPSED_FORMAT_LEN, elapsedTimeS);
}


//-- static versions
EXPORT void SgetTimestamp(double time_, char* oTimeS_) {
	char* ftime;
	time_t ltime; // calendar time 
	HANDLE TimeMutex;

	TimeMutex = CreateMutex(NULL, TRUE, NULL);
	WaitForSingleObject(TimeMutex, INFINITE);
	ltime = time(NULL); // get current cal time 
	ftime = asctime(localtime(&ltime));
	ftime[strlen(ftime)-1] = '\0';
	ReleaseMutex(TimeMutex);

	strcpy_s(oTimeS_, TIMER_TIME_FORMAT_LEN, ftime);

}
EXPORT void SgetElapsed(double time_, char* oTimeS_) {	//-- milliseconds to timestring (HH:MI:SS.ms)
	int ms = (int)time_%1000;
	int ss = (int)(floor(time_/1000))%60;
	int mi = (int)(floor(time_/1000/60))%60;
	int hh = (int)(floor(time_/1000/60/60))%60;
	sprintf_s(oTimeS_, 30, "%02d:%02d:%02d.%d", hh, mi, ss, ms);
}