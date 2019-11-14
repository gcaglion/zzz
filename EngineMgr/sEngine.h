#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/sTS2.h"
#include "../Logger/sLogger.h"
#include "sCore.h"
#include "sCoreLayout.h"

#define MAX_ENGINE_CORES	32

struct sEngine : sCfgObj {

	int pid;
	int type;
	int coresCnt;
	int* coreType;
	int* coreThreadId;
	int* coreLayer;
	sCore** core;

	/*int sampleLen;
	int targetLen;
	int featuresCnt;
	int batchSize;
	int WTtype;
	int WTlevel;*/

	numtype* forecast;

	EXPORT sEngine(sCfgObjParmsDef);
	EXPORT sEngine(sObjParmsDef, sLogger* persistor_, int clientPid_, int savedEnginePid_);
	EXPORT ~sEngine();

	EXPORT void infer(int simulationId_, int seqId_, sTS2* inferTS_, int savedEnginePid_);
	EXPORT void commit();

private:
	void mallocs();
};