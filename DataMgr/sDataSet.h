#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sTimeSerie.h"

#define VAL 0
#define TRVAL 1
#define TRSVAL 2

//-- DataSet section
#define DSsample		0
#define DStarget		1
#define DSprediction	2
//-- ordering Ids
#define SBForder	0
#define BFSorder	1
#define SFBorder	2

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
	numtype* sampleSBF;
	numtype* targetSBF;
	numtype* predictionSBF;
	//-- network training requires BFS ordering
	numtype* sampleBFS;
	numtype* targetBFS;
	numtype* predictionBFS;
	//-- array of pointers to any of the above : [Section][order]
	numtype* _data[3][2];

	EXPORT sDataSet(sObjParmsDef, sTimeSerie* sourceTS_, int sampleLen_, int predictionLen_, int batchSamplesCnt_, int selectedFeaturesCnt_, int* selectedFeature_, bool BWcalc_, int* BWfeature_=nullptr, const char* dumpPath_=nullptr);
	EXPORT sDataSet(sCfgObjParmsDef, int sampleLen_, int predictionLen_);
	EXPORT ~sDataSet();

	EXPORT void build(float scaleMin_=0, float scaleMax_=0, int type=TRSVAL);
	EXPORT void dump(int type=TRSVAL, bool prediction_=false);
	EXPORT void reorder(int section, int FROMorderId, int TOorderId);

private:
	void mallocs1();
	void mallocs2();
	void frees();
	bool isSelected(int ts_f);

	int BWfeaturesCnt=2;
	bool doDump;
	char* dumpPath;
};
