#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sTS.h"

struct sDS : sCfgObj {

	int sampleLen;
	int targetLen;
	int featuresCnt;
	int samplesCnt;

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

	EXPORT sDS(sCfgObjParmsDef);
	EXPORT sDS(sObjParmsDef, int parentDScnt_, sDS** parentDS_);
	EXPORT ~sDS();

	EXPORT void dump();
	EXPORT void scale(float scaleMin_, float scaleMax_);
	EXPORT void unscale();

private:
	void dumpPre(FILE** dumpFile);
};