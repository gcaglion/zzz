#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sTimeSerie.h"

struct sDS : sCfgObj {

	//-- common
	int featuresCnt;
	int patternLen;
	int patternsCnt;
	numtype* pattern;
	numtype* seqval;
	//--
	bool doDump;
	char dumpPath[MAX_PATH];

	//-- scaling info [featuresCnt]
	numtype* minVal;
	numtype* maxVal;
	numtype* scaleM;
	numtype* scaleP;

	//-- constructor 1: build from timeserie sequence
	EXPORT sDS(sObjParmsDef, int featuresCnt_, int stepsCnt_, numtype* sequenceBF_, int patternLen_, bool doDump_, char* dumpPath_);
	//-- constructor 2: build by merging existing datasets
	EXPORT sDS(sObjParmsDef, int parentDScnt_, sDS** parentDS_);
	//-- constructor 3: build from configuration file
	EXPORT sDS(sCfgObjParmsDef);
	//-- destructor
	EXPORT ~sDS();

	EXPORT void scale(float scaleMin_, float scaleMax_);
	EXPORT void unscale();

	EXPORT void setSequence();
	EXPORT void dumpPre(FILE** dumpFile);
	EXPORT void dump();

	EXPORT void setMinMax();
private:
	void mallocs();
};

