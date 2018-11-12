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

	sNNparms(sCfgObjParmsDef);
	sNNparms(sObjParmsDef);
	~sNNparms();

	EXPORT void setScaleMinMax();

private:
	void mallocs() {
		levelRatio=(float*)malloc((CORE_MAX_INTERNAL_LEVELS-2)*sizeof(float));
		ActivationFunction=(int*)malloc(CORE_MAX_INTERNAL_LEVELS*sizeof(int));
	}
};