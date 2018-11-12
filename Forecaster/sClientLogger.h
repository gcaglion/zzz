#pragma once

#include "../common.h"
#include "../Logger/sLogger.h"

struct sClientLogger : sLogger {

	//-- What
	bool saveClientInfoFlag;

	EXPORT sClientLogger(sCfgObjParmsDef);
	EXPORT ~sClientLogger();

	EXPORT void saveClientInfo(int pid, const char* clientName, double startTime, double elapsedSecs, int simulLen, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doTrainRun, bool doTestRun);

};