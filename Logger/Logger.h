#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"

// Logs Destinations
#define LOG_TO_TEXT   1
#define LOG_TO_ORCL	  2

typedef struct sLogger : sObj {
	int dest;
	bool saveNothing;
	bool saveClient;
	bool saveMSE;
	bool saveRun;
	bool saveInternals;
	bool saveImage;

	sLogger(sObjParmsDef, int dest_, bool saveNothing_=false, bool saveClient_=true, bool saveMSE_=true, bool saveRun_=true, bool saveInternals_=false, bool saveImage_=false);
	~sLogger();

	EXPORT void SaveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV) {}
	EXPORT void SaveRun(int pid, int tid, int setid, int npid, int ntid, int runCnt, int featuresCnt, int* feature, numtype* prediction, numtype* actual) {}
	EXPORT void SaveW(int pid, int tid, int epoch, int Wcnt, numtype* W) {}
	EXPORT void LoadW(int pid, int tid, int epoch, int Wcnt, numtype* W) {}
	EXPORT void SaveClient(int pid, char* clientName, DWORD startTime, DWORD duration, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun) {}
	EXPORT void Commit() {}

} tLogger;