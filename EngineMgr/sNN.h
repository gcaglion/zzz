#pragma once

#include "..\common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../Algebra/Algebra.h"
#include "../DataMgr/sDS.h"
#include "sCore.h"
#include "sNNparms.h"
#include "sNNenums.h"
#include "sSCGD.h"

struct sNN : sCore {

	EXPORT sNN(sObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sCoreLogger* persistor_, sNNparms* NNparms_);
	EXPORT sNN(sCfgObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sNNparms* NNparms_);
	EXPORT ~sNN();

	//-- local implementations of sCore virtual methods
	void setLayout(int batchSamplesCnt_);
	void mallocLayout();
	void train(sCoreProcArgs* trainArgs);
	void infer(sCoreProcArgs* inferArgs);
	void saveImage(int pid, int tid, int epoch);
	void loadImage(int pid, int tid, int epoch);

private:

	//-- NNParms
	sNNparms* parms;
	int pid, tid, testid;	//-- these are set from either procArgs or runArgs

	//-- internal layout
	int* nodesCnt;
	int nodesCntTotal;
	int outputLevel;
	int* levelFirstNode;
	int* ctxStart;
	int* weightsCnt;
	int weightsCntTotal;
	int* levelFirstWeight;

	//-- error measuring
	numtype* tse;	// total squared error.	Scalar. On GPU (if used)
	numtype* se;	// squared sum error.	Scalar. On GPU (if used)
	numtype tse_h;

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
	//--
	numtype* sample_d;
	numtype* target_d;
	//-- SCGD-specific
	sSCGD* scgd;


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
	void sNNcommon(sNNparms* NNparms_);

	void setCommonLayout();
	void FF();
	void Activate(int level);
	void resetBias();

	void Ecalc();
	void dEcalc();
	void EcalcG(sDS* ds, numtype* inW, numtype* outE);
	void dEcalcG(sDS* ds, numtype* inW, numtype* outdE);

	void loadBatchData(sDS* ds, int b);
	void ForwardPass(sDS* ds, int batchId, bool inferring);
	//bool epochSummary(int epoch, DWORD starttime, bool displayProgress=true);
	void showEpochStats(int e, DWORD eStart_);
	void showEpochStatsG(int e, DWORD eStart_, bool success_, numtype rnorm_);

	void BP_std();
	void WU_std();

	void BackwardPass(sDS* ds, int batchId, bool updateWeights);
	//-- malloc + init
	void mallocNeurons();
	void initNeurons();
	void createWeights();
	//-- free
	void destroyNeurons();
	void destroyWeights();
	void loadWholeDataSet();

	int trainSCGD(sCoreProcArgs* trainArgs);
};

