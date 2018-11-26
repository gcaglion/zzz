#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../EngineMgr/sCoreParms.h"
#include "sNNenums.h"

struct sNNparms : sCoreParms {

	//-- topology
	float* levelRatio;
	int* ActivationFunction;	// can be different for each level

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
	//-- training-BP_scgd specific
	int scgdMaxK;

	EXPORT sNNparms(sCfgObjParmsDef);
	EXPORT sNNparms(sObjParmsDef, sLogger* persistor_, int loadingPid_, int loadingTid_);
	EXPORT ~sNNparms();

	//-- local implementations of virtual functions defined in sCoreParms
	EXPORT void setScaleMinMax();
	EXPORT void save(sLogger* persistor_, int pid_, int tid_);

private:
	void mallocs();
	char* levelRatioS;
	char* levelActivationS;
};