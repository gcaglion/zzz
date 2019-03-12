#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/sDataSet.h"
#include "../DataMgr/newDS.h"
#include "sCore.h"
#include "sCoreParms.h"
#include "Engine_enums.h"
#include "sNN.h"
#include "sGA.h"
#include "sSVM.h"
#include "sSOM.h"
#include "sDUMB.h"

#define MAX_ENGINE_LAYERS	8
#define MAX_ENGINE_CORES	32

struct sEngine : sCfgObj {

	int coresCnt;
	int layersCnt=0;
	int* layerCoresCnt;

	sAlgebra* Alg;
	sLogger* persistor;

	sDataShape* shape;
	sCore** core;
	sCoreLayout** coreLayout;
	sCoreParms** coreParms;
	sCoreLogger** corePersistor;

	EXPORT sEngine(sObjParmsDef, sLogger* fromPersistor_, int clientPid_, int loadingPid_);
	EXPORT sEngine(sCfgObjParmsDef, sDataShape* shape_, int clientPid_);
	EXPORT ~sEngine();

	EXPORT void train2(int testid_, sDS* sampleDS_, sDS* targetDS_, sDS* predictionDS_, int batchSize_);
	EXPORT void train(int testid_, sDataSet* trainDS_);
	EXPORT void infer(int testid_, sDataSet* inferDS_, int savedEnginePid_, bool reTransform=true);
	//--
	EXPORT void saveMSE();
	EXPORT void saveRun();
	EXPORT void saveCoreImages(int epoch=-1);
	EXPORT void saveCoreLoggers();
	//--
	EXPORT void saveInfo();
	//--
	EXPORT void commit();

private:
	int clientPid;
	sNN* NNc; sGA* GAc; sSVM* SVMc; sSOM* SOMc; sDUMB* DUMBc;
	sNNparms* NNcp; sGAparms* GAcp; sSVMparms* SVMcp; sSOMparms* SOMcp; sDUMBparms* DUMBcp;
	void spawnCoresFromXML();
	void spawnCoresFromDB(int loadingPid);
	void setCoreLayer(sCoreLayout* cl);
	void setLayerProps();
	const int trainProc = 0;
	const int inferProc = 1;
	void process(int procid_, bool loadImage_, int testid_, sDataSet* ds_, int batchSize_, int savedEnginePid_);

	//-- these are needed to save trmin/max for each training feature
	sDataSet* trainDS;
	//-- these are needed to load trmin/max for each training feature
	int sourceTSCnt;
	int* TSfeaturesCnt;
	int** TSfeature;
	numtype** TStrMin;
	numtype** TStrMax;
	//--
	void mallocTSinfo();
	void freeTSinfo();

};
