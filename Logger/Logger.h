#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfgObj.h"
#include "sFileDB.h" 
#include "../OraOci/sOraDB.h"

typedef struct sLogger : sCfgObj {
	sOraDB* db;
	FILE* fileH[5];
	char** fileFullName;	// one file for each log (Client, MSE, Run, Internals, Image)
	bool saveNothingFlag;
	bool saveClientFlag;
	bool saveMSEFlag;
	bool saveRunFlag;
	bool saveInternalsFlag;
	bool saveImageFlag;

	//EXPORT sLogger(sObjParmsDef, sOraConnection* db_, bool saveNothing_=false, bool saveClient_=true, bool saveMSE_=true, bool saveRun_=true, bool saveInternals_=false, bool saveImage_=false);
	//EXPORT sLogger(sObjParmsDef, FILE* fileH_, bool saveNothing_=false, bool saveClient_=true, bool saveMSE_=true, bool saveRun_=true, bool saveInternals_=false, bool saveImage_=false);
	EXPORT sLogger(sCfgObjParmsDef);
	EXPORT ~sLogger();

	EXPORT void saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV);
	EXPORT void saveRun(int pid, int tid, int setid, int npid, int ntid, int runCnt, int featuresCnt, int* feature, numtype* prediction, numtype* actual);
	EXPORT void saveW(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadW(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveClient(int pid, char* clientName, DWORD startTime, DWORD duration, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun);
	EXPORT void commit();

private:
	void ffname_malloc();
} tLogger;