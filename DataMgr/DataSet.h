#pragma once

#include "../common.h"
#include "../configMgr/sCfgObj.h"
#include "../configMgr/sCfgKey.h"
#include "TimeSerie.h"

typedef struct sDataSet : sCfgObj {

	tTimeSerie* sourceTS=nullptr;

	int sampleLen;
	int targetLen;
	int selectedFeaturesCnt;
	int* selectedFeature;
	int* BWFeature;

	int samplesCnt;
	int batchSamplesCnt;
	int batchCnt;

	//-- sample, target, prediction are stored in  order (Sample-Bar-Feature)
	numtype* sample=nullptr;
	numtype* target=nullptr;
	numtype* prediction=nullptr;
	//-- network training requires BFS ordering
	numtype* sampleBFS=nullptr;
	numtype* targetBFS=nullptr;
	numtype* predictionBFS=nullptr;
	//-- network inference requires SFB ordering to get first-step prediction
	numtype* targetSFB=nullptr;
	numtype* predictionSFB=nullptr;
	//-- one-step only target+prediction (required by run() ) ???????
	numtype* target0=nullptr;
	numtype* prediction0=nullptr;

	//-- constructor / destructor
	EXPORT sDataSet(sCfgObjParmsDef, int sampleLen_, int targetLen_, int batchSamplesCnt_, int selectedFeaturesCnt_, int* selectedFeature_, int* datafileBWFeature_);
	EXPORT sDataSet(sCfgObjParmsDef);
	EXPORT ~sDataSet();

	Bool isSelected(int ts_f);
	EXPORT void buildFromTS();
	EXPORT void SBF2BFS(int batchId, int barCnt, numtype* fromSBF, numtype* toBFS);
	EXPORT void BFS2SBF(int batchId, int barCnt, numtype* fromBFS, numtype* toSBF);
	EXPORT void BFS2SFB(int batchId, int barCnt, numtype* fromBFS, numtype* toSFB);
	EXPORT void BFS2SFBfull(int barCnt, numtype* fromBFS, numtype* toSFB);
	EXPORT void dump(char* dumpFileName=nullptr);

private:
	void sDataSet_pre();
	void sDataSet_post();
	void sDataSet_common();
	void sDataSet_malloc();
} tDataSet;