#pragma once

#include "..\common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../Algebra/Algebra.h"
#include "../DataMgr/sDataSet.h"
#include "sCore.h"
#include "sNNparms.h"
#include "sNNenums.h"

struct sNN : sCore {

	EXPORT sNN(sCfgObjParmsDef, sCoreLayout* layout_, sNNparms* NNparms_);
	EXPORT ~sNN();

	EXPORT void train(sDataSet* trainSet);
	EXPORT void run(sDataSet* runSet);


private:
	//-- MyAlgebra common structures
	sAlgebra* Alg;

	//-- every instantiation has 1 process id and 1 thread id (TO BE CONFIRMED)
	int pid;
	int tid;

	//-- NNParms
	sNNparms* parms;

	int* nodesCnt;
	int nodesCntTotal;
	int outputLevel;
	int* levelFirstNode;
	int* ctxStart;
	int* weightsCnt;
	int weightsCntTotal;
	int* levelFirstWeight;

	//-- training stuff
	int ActualEpochs;
	//-- error measuring
	numtype* tse;	// total squared error.	Scalar. On GPU (if used)
	numtype* se;	// squared sum error.	Scalar. On GPU (if used)
	numtype* mseT;	// Training mean squared error, array indexed by epoch, always on host
	numtype* mseV;	// Validation mean squared error, array indexed by epoch, always on host

	//-- set at each level according to ActivationFunction
	float* scaleMin;	
	float* scaleMax;

	numtype* a;
	numtype* F;
	numtype* dF;
	numtype* edF;
	numtype* W;
	numtype* prevW;
	numtype* dW;
	numtype* dJdW;
	numtype* e;
	numtype* u;


	//-- performance counters
	DWORD LDstart, LDtimeTot=0, LDcnt=0; float LDtimeAvg;
	DWORD FFstart, FFtimeTot=0, FFcnt=0; float FFtimeAvg;
	DWORD FF0start, FF0timeTot=0, FF0cnt=0; float FF0timeAvg;
	DWORD FF1start, FF1timeTot=0, FF1cnt=0; float FF1timeAvg;
	DWORD FF1astart, FF1atimeTot=0, FF1acnt=0; float FF1atimeAvg;
	DWORD FF1bstart, FF1btimeTot=0, FF1bcnt=0; float FF1btimeAvg;
	DWORD FF2start, FF2timeTot=0, FF2cnt=0; float FF2timeAvg;
	DWORD CEstart, CEtimeTot=0, CEcnt=0; float CEtimeAvg;
	DWORD VDstart, VDtimeTot=0, VDcnt=0; float VDtimeAvg;
	DWORD VSstart, VStimeTot=0, VScnt=0; float VStimeAvg;
	DWORD BPstart, BPtimeTot=0, BPcnt=0; float BPtimeAvg;
	DWORD WUstart, WUtimeTot=0, WUcnt=0; float WUtimeAvg;
	DWORD TRstart, TRtimeTot=0, TRcnt=0; float TRtimeAvg;

	//--
	void setCommonLayout();
	void setLayout(int batchSamplesCnt_);
	void FF();
	void Activate(int level);
	void calcErr();
	void ForwardPass(sDataSet* ds, int batchId, bool haveTargets);
	bool epochMetCriteria(int epoch, DWORD starttime, bool displayProgress=true);
	void BP_std();
	void WU_std();
	void BackwardPass(tDataSet* ds, int batchId, bool updateWeights);
	//-- malloc + init
	void mallocNeurons();
	void initNeurons();
	void createWeights();
	//-- free
	void destroyNeurons();
	void destroyWeights();

private:
	int _batchCnt;
	int _batchSize;
};

