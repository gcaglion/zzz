#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/sDS.h"
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

	int type;

	int WNNdecompLevel;
	int WNNwaveletType;

	int coresCnt;
	int layersCnt=0;
	int* layerCoresCnt;

	sAlgebra* Alg;
	sLogger* persistor;

	int sampleLen;
	int targetLen;
	int featuresCnt;

	sCore** core;
	sCoreLayout** coreLayout;
	sCoreParms** coreParms;
	sCoreLogger** corePersistor;
	sEngineProcArgs** procArgs;

	EXPORT sEngine(sObjParmsDef, sLogger* fromPersistor_, int clientPid_, int loadingPid_);
	EXPORT sEngine(sCfgObjParmsDef, int sampleLen_, int targetLen_, int featuresCnt_, int clientPid_);
	EXPORT ~sEngine();

	EXPORT void train(int testid_, sTS* trainTS_, int sampleLen, int targetLen_, int batchSize_);
	EXPORT void infer(int testid_, sDS* inferDS_, int savedEnginePid_, bool reTransform=true);
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
	void process(int procid_, bool loadImage_, int testid_, sDS** ds_, int savedEnginePid_);

	//-- these are needed to save trmin/max for each training feature
	sDS* trainDS;
	numtype* DStrMin; numtype* DStrMax;


};
