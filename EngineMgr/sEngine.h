#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/sDataShape.h"
#include "../DataMgr/sDataSet.h"
#include "sCore.h"
#include "sCoreParms.h"
#include "Engine_enums.h"
#include "sNN.h"
#include "sGA.h"
#include "sSVM.h"
#include "sSOM.h"

#define MAX_ENGINE_LAYERS	8
#define MAX_ENGINE_CORES	32

struct sEngineHandle {
	int ProcessId;
	int TestId;
	//int DatasetId;
	int ThreadId;
};

struct sTrainParams {
	//sEngine* EngineParms;
	int LayerId; int CoreId; int DatasetId;
	int CorePos; int TotCores; HANDLE ScreenMutex;
	bool useValidation;
	bool useExistingW;
	int SampleCount; double** SampleT; double** TargetT; double** SampleV; double** TargetV;
	sEngineHandle TrainInfo;
	int TrainSuccess;
	int ActualEpochs;
};

struct sEngine : sCfgObj {

	int type;
	int coresCnt;
	int layersCnt=0;
	int* layerCoresCnt;

	sDataShape* dataShape;
	sCore** core;
	sCoreLayout** coreLayout;
	sCoreParms** coreParms;

	EXPORT sEngine(sCfgObjParmsDef, sDataShape* dataShape_);
	EXPORT ~sEngine();

	EXPORT void train(int testid_, sDataSet* trainDS_);
	EXPORT void infer(int testid_, sDataSet* testDS_);
	EXPORT void saveMSE();

private:
	int pid;
	void setCoreLayer(sCoreLayout* cl);
};
