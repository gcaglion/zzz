#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../EngineMgr/sCoreParms.h"
#include "sNNenums.h"

struct sNNparms : sCoreParms {

	//-- topology
	//int levelsCnt;
	float* levelRatio;
	int* ActivationFunction;	// can be different for each level
	//int batchSamplesCnt;	// usually referred to as Batch Size
	bool useContext;
	bool useBias;

	//-- training-common
	int MaxEpochs;
	float TargetMSE;
	int NetSaveFreq;
	bool StopOnDivergence;
	int BP_Algo;
	//-- training-BP_Std specific
	float LearningRate;
	float LearningMomentum;

	EXPORT sNNparms(sCfgObjParmsDef);
	EXPORT sNNparms(sObjParmsDef, sLogger* persistor_, int loadingPid_);
	EXPORT ~sNNparms();

	//-- local implementations of virtual functions defined in sCoreParms
	EXPORT void setScaleMinMax();
	EXPORT void save(int pid, int tid);
	EXPORT void load(int pid, int tid);

private:
	void mallocs();
};