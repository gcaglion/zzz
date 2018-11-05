#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"

#define MT4_MODE_READ	0
#define MT4_MODE_WRITE	1
#define MT4_MODE_APPEND	2

struct sMT4Data : sCfgObj {

	int mt4sCnt;
	char** mt4FullName;	// one mt4 for each log (Client, MSE, Run, Internals, Image)
	FILE** mt4H;

	EXPORT sMT4Data(sObjParmsDef, int openMode_, bool autoOpen_);
	EXPORT sMT4Data(sCfgObjParmsDef);
	EXPORT ~sMT4Data();

	EXPORT void open(int mode_);
	EXPORT void close();
	EXPORT void getStartDates(sMT4Data* dateSource_, char* startDate_, int datesCnt_, char** oDate_);

	EXPORT void saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV);
	EXPORT void saveRun(int pid, int tid, int npid, int ntid, int barsCnt, int featuresCnt, int* feature, int predictionLen, numtype* actual, numtype* predicted);
	EXPORT void saveW(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadW(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveClient(int pid, char* clientName, DWORD startTime, DWORD duration, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun);
	EXPORT void commit();

private:
	void mallocs();

};