#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../FileData/sFileData.h" 
#include "../OraData/sOraData.h"

struct sLogger : sCfgObj {
	sOraData* db;
	sFileData* file;

	char** fileFullName;	// one file for each log (Client, MSE, Run, Internals, Image)
	bool saveNothingFlag;
	bool saveClientFlag;
	//--

	EXPORT sLogger(sCfgObjParmsDef);
	EXPORT ~sLogger();

	EXPORT void loadW(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveClient(int pid, char* clientName, DWORD startTime, DWORD duration, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun);
	EXPORT void commit();

private:
	void ffname_malloc();
};