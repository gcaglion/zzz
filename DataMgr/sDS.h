#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sTS.h"

#define TARGET 0
#define PREDICTION 1

struct sDS : sCfgObj {

	int sampleLen;
	int targetLen;
	int featuresCnt;
	int samplesCnt;
	int batchSize;

	bool doDump;
	char dumpPath[MAX_PATH];

	//-- sample, target, prediction are stored in  order (Sample-Bar-Feature)
	numtype* sampleSBF;
	numtype* targetSBF;
	numtype* predictionSBF;
	//-- network training requires BFS ordering
	numtype* sampleBFS;
	numtype* targetBFS;
	numtype* predictionBFS;

	numtype* trmin;
	numtype* trmax;
	numtype* scaleM;
	numtype* scaleP;

	numtype* seqBase;
	int seqLen;
	char** seqLabel;
	int seqDT;

	EXPORT sDS(sCfgObjParmsDef);
	EXPORT sDS(sObjParmsDef, int parentDScnt_, sDS** parentDS_);
	EXPORT sDS(sObjParmsDef, sDS* copyFromDS_);
	EXPORT sDS(sObjParmsDef, sTS* fromTS_, int WNNsrc_, int sampleLen_, int targetLen_, int batchSize_, bool doDump_, char* dumpPath_=nullptr);
	EXPORT ~sDS();

	EXPORT void dump();
	EXPORT void scale(float scaleMin_, float scaleMax_);
	EXPORT void unscale();
	EXPORT void getSeq(int trg_vs_prd, numtype* oVal);
	EXPORT void untransformSeq(numtype* iTRval, numtype* iActualVal, numtype* oBASEval);

	EXPORT void target2prediction();

	EXPORT void setBFS(int batchCnt, int batchSize);
	EXPORT void setSBF(int batchCnt, int batchSize);
private:
	void SBF2BFS(int batchSamplesCnt, int batchId, int barCnt, numtype* fromSBF, numtype* toBFS);
	void BFS2SBF(int batchSamplesCnt, int batchId, int barCnt, numtype* fromBFS, numtype* toSBF);
	void dumpPre(FILE** dumpFile);
	void mallocs1();
	void buildFromTS(sTS* ts_, int WNNsrc_);
};