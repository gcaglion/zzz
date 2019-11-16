#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/sTS2.h"
#include "../Logger/sLogger.h"
#include "sCore.h"
#include "sCoreLayout.h"
#include "sNNparms.h"
#include "sNN2.h"

#define MAX_ENGINE_CORES	32

struct sEngine : sCfgObj {

	sLogger* persistor;

	int pid;
	int type;
	int coresCnt;
	int* coreType;
	int* coreThreadId;
	int* coreLayer;

	sCore** core;

	int sampleLen;
	int targetLen;
	int batchSize;
	int samplesCnt;
	int inputCnt;
	int outputCnt;

	int dataSourcesCnt[2];
	int* featuresCnt[2];
	int WTlevel[2];
	int WTtype[2];

	numtype* sample;
	numtype* target;
	numtype* prediction;
	numtype* trMinIN=nullptr;
	numtype* trMaxIN=nullptr;
	numtype* trMinOUT=nullptr;
	numtype* trMaxOUT=nullptr;

	numtype* forecast;

	EXPORT sEngine(sCfgObjParmsDef);
	EXPORT sEngine(sObjParmsDef, sLogger* persistor_, int clientPid_, int savedEnginePid_);
	EXPORT ~sEngine();

	EXPORT void train(int simulationId_, sTS2* trainTS_, int sampleLen_, int targetLen_, int batchSize_);
	EXPORT void infer(int simulationId_, int seqId_, sTS2* inferTS_, int savedEnginePid_);
	EXPORT void commit();

private:
	void mallocs();
};