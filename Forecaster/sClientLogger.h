#pragma once

#include "../common.h"
#include "../Logger/sLogger.h"

struct sClientLogger : sLogger {

	//-- What
	bool saveClientInfoFlag;

	EXPORT sClientLogger(sCfgObjParmsDef);
	EXPORT ~sClientLogger();

	EXPORT void saveClientInfo(int pid, char* clientName, double startTime, double elapsedSecs, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun);

};