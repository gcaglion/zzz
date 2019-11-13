#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sTS.h"

#define TARGET 0
#define PREDICTION 1

struct sDS : sCfgObj {

	int sampleLen;
	int targetLen;
	int featuresCnt;
	int WTlevel;
	int samplesCnt;
	int batchSize;

	bool doDump;
	char dumpPath[MAX_PATH];

	//-- sample, target, prediction are stored in  order (Sample-Bar-Feature)
	numtype* sample;
	numtype* target;
	numtype* prediction;

	numtype* TRmin;
	numtype* TRmax;
	numtype* scaleM;
	numtype* scaleP;

	EXPORT sDS(sCfgObjParmsDef);
	EXPORT sDS(sObjParmsDef, int parentDScnt_, sDS** parentDS_);
	EXPORT sDS(sObjParmsDef, sDS* copyFromDS_);
	EXPORT sDS(sObjParmsDef, sTS* fromTS_, int sampleLen_, int targetLen_, int batchSize_, bool doDump_, char* dumpPath_=nullptr);
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
	EXPORT void getSeq(int trg_vs_prd, numtype* oVal);
	EXPORT void untransformSeq(int seqDT_, numtype* seqBase_, numtype* iTRval, numtype* iActualVal, numtype* oBASEval);

	EXPORT void target2prediction();

private:
	void dumpPre(bool isScaled, FILE** dumpFile);
	void mallocs1();
	void buildFromTS(sTS* ts_);
};