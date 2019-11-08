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
#include "sEncoder.h"


#define MAX_ENGINE_LAYERS	8
#define MAX_ENGINE_CORES	32

struct sEngine : sCfgObj {

	int type;

	int WTlevel;
	int WTtype;

	int coresCnt;
	int layersCnt=0;
	int* layerCoresCnt;

	sLogger* persistor;

	int sampleLen;
	int targetLen;
	int featuresCnt;
	int batchSize;

	sCore** core;
	sCoreLayout** coreLayout;
	sCoreParms** coreParms;
	sCoreLogger** corePersistor;
	sEngineProcArgs** procArgs;

	numtype* forecast;

	EXPORT sEngine(sObjParmsDef, sLogger* fromPersistor_, int clientPid_, int loadingPid_);
	EXPORT sEngine(sCfgObjParmsDef, int sampleLen_, int targetLen_, int featuresCnt_, int WTlevel_, int WTtype_, int batchSize_, int clientPid_);
	EXPORT ~sEngine();

	void loadImage(int loadingPid_);
	EXPORT void train(int testid_, sDS** trainDS_);
	EXPORT void infer(int testid_, int seqId_, sDS** trainDS_, sTS* inferTS_, int savedEnginePid_);
	EXPORT void infer(int testid_, int seqId_, sDS* inferDS_, int batchSize_);
	//--
	EXPORT void saveInfo();
	//--
	EXPORT void commit();

	//-- these are needed to save trmin/max for each training feature
	//sDS* trainDS;
	numtype* DStrMin; numtype* DStrMax;
	numtype** DSfftMin; numtype** DSfftMax;

	const int trainProc = 0;
	const int inferProc = 1;
	const int loadProc = 2;

private:
	int clientPid;
	sNN* NNc; sGA* GAc; sSVM* SVMc; sSOM* SOMc; sDUMB* DUMBc;
	sNNparms* NNcp; sGAparms* GAcp; sSVMparms* SVMcp; sSOMparms* SOMcp; sDUMBparms* DUMBcp;
	void spawnCoresFromXML();
	void spawnCoresFromDB(int loadingPid);
	void setCoreLayer(sCoreLayout* cl);
	void setLayerProps();
	bool imageLoaded=false;
	void process(int procid_, int testid_, sDS** ds_, int savedEnginePid_);


};
