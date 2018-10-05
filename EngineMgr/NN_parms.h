#pragma once
#include "../common.h"

#define MAX_LEVELS 128

struct sNNparms {

	//-- topology
	int levelsCnt;
	float* levelRatio;
	int* ActivationFunction;	// can be different for each level
	int batchSamplesCnt;	// usually referred to as Batch Size
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

	sNNparms() {
		levelRatio=(float*)malloc((MAX_LEVELS-2)*sizeof(float));
		ActivationFunction=(int*)malloc(MAX_LEVELS*sizeof(int));
	}
	~sNNparms() {
		free(levelRatio);
		free(ActivationFunction);
	}

};