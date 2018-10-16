#pragma once

#include "../common.h"
#include "../configMgr/sCfgObj.h"
#include "../configMgr/sCfgKey.h"
#include "sDataShape.h"
#include "sTimeSerie.h"

typedef struct sDataSet : sCfgObj {

	sDataShape* shape;

	tTimeSerie* sourceTS;

	int selectedFeaturesCnt;
	int* selectedFeature;
	int* BWFeature;

	int samplesCnt;
	int batchSamplesCnt;
	int batchCnt;

	//-- sample, target, prediction are stored in  order (Sample-Bar-Feature)
	numtype* sample;
	numtype* target;
	numtype* prediction;
	//-- network training requires BFS ordering
	numtype* sampleBFS;
	numtype* targetBFS;
	numtype* predictionBFS;
	//-- network inference requires SFB ordering to get first-step prediction
	numtype* targetSFB;
	numtype* predictionSFB;
	//-- one-step only target+prediction (required by run() ) ???????
	numtype* target0;
	numtype* prediction0;

	//-- constructor / destructor
	EXPORT sDataSet(sCfgObjParmsDef, sDataShape* shape_, int batchSamplesCnt_, int selectedFeaturesCnt_, int* selectedFeature_, int* datafileBWFeature_);
	EXPORT sDataSet(sCfgObjParmsDef, sDataShape* shape_);
	EXPORT ~sDataSet();

	bool isSelected(int ts_f);
	EXPORT void buildFromTS(float scaleMin_, float scaleMax_);
	EXPORT void SBF2BFS(int batchId, int barCnt, numtype* fromSBF, numtype* toBFS);
	EXPORT void BFS2SBF(int batchId, int barCnt, numtype* fromBFS, numtype* toSBF);
	EXPORT void BFS2SFB(int batchId, int barCnt, numtype* fromBFS, numtype* toSFB);
	EXPORT void BFS2SFBfull(int barCnt, numtype* fromBFS, numtype* toSFB);
	EXPORT void dump();

private:
	void sDataSet_pre();
	void sDataSet_post();
	char* dumpFileFullName;

} tDataSet;