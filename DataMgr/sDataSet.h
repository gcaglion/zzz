#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sTimeSerie.h"
#include "../Algebra/Algebra.h"

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
	bool isCloned;

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
	numtype** sampleSBF;		//--[BASE/TR/TRS]
	numtype** targetSBF;		//--[BASE/TR/TRS]
	numtype** predictionSBF;	//--[BASE/TR/TRS]
	//-- network training requires BFS ordering
	numtype** sampleBFS;		//--[BASE/TR/TRS]
	numtype** targetBFS;		//--[BASE/TR/TRS]
	numtype** predictionBFS;	//--[BASE/TR/TRS]
	//-- array of pointers to any of the above : [Section][order]
	numtype** _data[3][2];

	EXPORT sDataSet(sObjParmsDef, sTimeSerie* sourceTS_, int sampleLen_, int predictionLen_, int batchSamplesCnt_, int selectedFeaturesCnt_, int* selectedFeature_, bool BWcalc_, int* BWfeature_=nullptr, bool doDump=false, const char* dumpPath_=nullptr);
	EXPORT sDataSet(sCfgObjParmsDef, int sampleLen_, int predictionLen_);
	EXPORT sDataSet(sObjParmsDef, sDataSet* trainDS_);
	EXPORT ~sDataSet();

	EXPORT void build(int valStatus, int valSource);
	EXPORT void unbuild(int valStatus, int valSource);	//-- takes step 0 from predictionSBF, copy it into sourceTS->trsvalP

	EXPORT void reorder(int section, int FROMorderId, int TOorderId);

private:
	void mallocs();
	void mallocs1();
	void mallocs2();
	void frees();
	bool isSelected(int ts_f);
	void dumpPre(int valStatus, FILE** dumpFile);

	int BWfeaturesCnt=2;
	bool doDump;
	char* dumpPath;
};
