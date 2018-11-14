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

	EXPORT sNNparms(sCfgObjParmsDef);
	EXPORT sNNparms(sObjParmsDef, sLogger* persistor_, int loadingPid_);
	EXPORT ~sNNparms();

	//-- local implementations of virtual functions defined in sCoreParms
	EXPORT void setScaleMinMax();
	EXPORT void save(sLogger* persistor_, int pid_, int tid_);
	EXPORT void load(sLogger* persistor_, int pid_, int tid_);

private:
	void mallocs();
	char levelRatioS[XMLKEY_PARM_VAL_MAXLEN]; 
	char levelActivationS[XMLKEY_PARM_VAL_MAXLEN];
};