#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
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

struct sEngine : sCfgObj {

	int type;
	int inputCnt;
	int outputCnt;
	int coresCnt;
	int layersCnt=0;
	int* layerCoresCnt;

	sCore** core;
	sCoreLayout** coreLayout;
	sCoreParms** coreParms;

	EXPORT sEngine(sCfgObjParmsDef, int inputCnt_, int outputCnt_);
	EXPORT ~sEngine();

	EXPORT void train(int testid_, sDataSet* trainDS_);
	EXPORT void infer(int testid_, sDataSet* inferDS_);
	EXPORT void saveMSE();
	EXPORT void saveRun();
	EXPORT void commit();

private:
	int pid;
	void setCoreLayer(sCoreLayout* cl);
	const int trainProc = 0;
	const int inferProc = 1;
	void process(int procid_, int testid_, sDataSet* ds_);
};
