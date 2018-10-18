#pragma once

#include "../common.h"
#include "../Logger/sLogger.h"

struct sClientLogger : sLogger {

	//-- What
	bool saveClientInfoFlag;

	EXPORT sClientLogger(sCfgObjParmsDef);
	EXPORT ~sClientLogger();

	EXPORT void saveClientInfo(int pid, char* clientName, DWORD startTime, DWORD duration, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun);

};