#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/sTS2.h"
#include "../Algebra/Algebra.h"
#include "sNNenums.h"

struct sEncoder : sObj {
	int levelsCnt;
	int* nodesCnt;
	int* weightsCnt;
	int nodesCntTotal;
	int weightsCntTotal;
	int* levelFirstNode;
	int* levelFirstWeight;
	int* activationFunction;

	numtype learningRate;
	numtype learningMomentum;

	sAlgebra* Alg;

	numtype* a;
	numtype* F;
	numtype* dF;
	numtype* edF;
	numtype* er;
	numtype* W;
	numtype* dW;
	numtype* dJdW;

	numtype* sse;	//-- sample squared error
	numtype* bse;	//-- batch  squared error
	numtype* ese;	//-- epoch  squared error

	int pid, tid;

	EXPORT sEncoder(sObjParmsDef, int inputSize_, int levelsCnt_, float* levelRatio_, int* activationFunction_, numtype learningRate_, numtype learningMomentum_);
	EXPORT ~sEncoder();

	void sEncoder::FF();
	void sEncoder::Activate(int level);
	void sEncoder::dEcalc();
	void sEncoder::showEpochStats(int e, int maxe, DWORD eStart_, numtype epochMSE);

	EXPORT void train(int samplesCnt, int sampleLen, int featuresCnt, numtype* sample, int batchSize, int maxEpochs);

};
