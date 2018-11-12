#pragma once

#include "../common.h"
#include "../Logger/sLogger.h"

struct sClientLogger : sLogger {

	//-- What
	bool saveClientInfoFlag;

	EXPORT sClientLogger(sCfgObjParmsDef);
	EXPORT ~sClientLogger();

	EXPORT void saveClientInfo(int pid, int simulationId, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doTrainRun, bool doTestRun);

};