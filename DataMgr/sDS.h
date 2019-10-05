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

	numtype* TRmin;
	numtype* TRmax;
	numtype* scaleM;
	numtype* scaleP;

	EXPORT sDS(sCfgObjParmsDef);
	EXPORT sDS(sObjParmsDef, int parentDScnt_, sDS** parentDS_);
	EXPORT sDS(sObjParmsDef, sDS* copyFromDS_);
	EXPORT sDS(sObjParmsDef, sTS* fromTS_, int WNNsrc_, int sampleLen_, int targetLen_, int batchSize_, bool doDump_, char* dumpPath_=nullptr);
	EXPORT sDS(sObjParmsDef, const char* srcFileName_);
	EXPORT ~sDS();

	EXPORT void invertSequence();
	EXPORT void slideSequence(int steps);
	EXPORT void duplicateSequence();
	EXPORT void halveSequence();
	EXPORT void swapFirstLast();

	EXPORT void dump(bool isScaled=false);
	EXPORT void scale(float scaleMin_, float scaleMax_);
	EXPORT void unscale();
	EXPORT void getSeq(int trg_vs_prd, numtype* oVal, sDS* baseDS);
	EXPORT void untransformSeq(int seqDT_, numtype* seqBase_, numtype* iTRval, numtype* iActualVal, numtype* oBASEval);

	EXPORT void target2prediction();

	EXPORT void setBFS(int batchCnt, int batchSize);
	EXPORT void setSBF(int batchCnt, int batchSize);
private:
	void SBF2BFS(int batchSamplesCnt, int batchId, int barCnt, numtype* fromSBF, numtype* toBFS);
	void BFS2SBF(int batchSamplesCnt, int batchId, int barCnt, numtype* fromBFS, numtype* toSBF);
	void dumpPre(bool isScaled, FILE** dumpFile);
	void mallocs1();
	void buildFromTS(sTS* ts_, int WNNsrc_);
};