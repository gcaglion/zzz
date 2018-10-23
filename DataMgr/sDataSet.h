#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sTimeSerie.h"

#define VAL 0
#define TRVAL 1
#define TRSVAL 2

struct sDataSet : sCfgObj {

	sTimeSerie* sourceTS;

	int sampleLen;
	int predictionLen;

	int selectedFeaturesCnt;
	int* selectedFeature;
	bool BWcalc;
	int* BWfeature;

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

	EXPORT sDataSet(sObjParmsDef, sTimeSerie* sourceTS_, int sampleLen_, int predictionLen_, int batchSamplesCnt_, int selectedFeaturesCnt_, int* selectedFeature_, bool BWcalc_, int* BWfeature_=nullptr);
	EXPORT sDataSet(sCfgObjParmsDef, int sampleLen_, int predictionLen_);
	EXPORT ~sDataSet();

	EXPORT void build(float scaleMin_=0, float scaleMax_=0, int type=TRSVAL);
	EXPORT void dump(int type=TRSVAL);
	
	//-- this is called directly from sNN
	EXPORT void BFS2SFBfull(int barCnt, numtype* fromBFS, numtype* toSFB);

private:
	void mallocs1();
	void mallocs2();
	void frees();
	bool isSelected(int ts_f);
	void SBF2BFS(int batchId, int barCnt, numtype* fromSBF, numtype* toBFS);
	void BFS2SBF(int batchId, int barCnt, numtype* fromBFS, numtype* toSBF);
	void BFS2SFB(int batchId, int barCnt, numtype* fromBFS, numtype* toSFB);
	int BWfeaturesCnt=2;
	bool doDump;
};
