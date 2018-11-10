#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sTimeSerie.h"
#include "../Algebra/Algebra.h"

//-- DataSet section
#define DSsample		0
#define DStarget		1
#define DSprediction	2

//-- ordering Ids
#define SBF	0
#define BFS	1

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
	numtype* sampleSBF;		
	numtype* targetSBF;		
	numtype* predictionSBF;	
	//-- network training requires BFS ordering
	numtype* sampleBFS;		
	numtype* targetBFS;		
	numtype* predictionBFS;	
	//-- array of pointers to any of the above : [Section][order]
	numtype** _data[3][2];

	numtype***** val;	//-- [Source][Status][Sample][Order][len]
/*
val[SAMPLE][BASE]SBF][sampleId][sampleLen];
val[SAMPLE][TR]SBF][sampleId][sampleLen];
val[SAMPLE][TRS]SBF][sampleId][sampleLen];
val[TARGET][BASE]SBF][sampleId][sampleLen];
val[TARGET][TR]SBF][sampleId][sampleLen];
val[TARGET][TRS]SBF][sampleId][sampleLen];
val[PREDICTION][BASE]SBF][sampleId][sampleLen];
val[PREDICTION][TR]SBF][sampleId][sampleLen];
val[PREDICTION][TRS]SBF][sampleId][sampleLen];
val[SAMPLE][BASE]BFS][sampleId][sampleLen];
val[SAMPLE][TR]BFS][sampleId][sampleLen];
val[SAMPLE][TRS]BFS][sampleId][sampleLen];
val[TARGET][BASE]BFS][sampleId][sampleLen];
val[TARGET][TR]BFS][sampleId][sampleLen];
val[TARGET][TRS]BFS][sampleId][sampleLen];
val[PREDICTION][BASE]BFS][sampleId][sampleLen];
val[PREDICTION][TR]BFS][sampleId][sampleLen];
val[PREDICTION][TRS]BFS][sampleId][sampleLen];
*/

	EXPORT sDataSet(sObjParmsDef, sTimeSerie* sourceTS_, int sampleLen_, int predictionLen_, int batchSamplesCnt_, int selectedFeaturesCnt_, int* selectedFeature_, bool BWcalc_, int* BWfeature_=nullptr, bool doDump=false, const char* dumpPath_=nullptr);
	EXPORT sDataSet(sCfgObjParmsDef, int sampleLen_, int predictionLen_);
	EXPORT sDataSet(sObjParmsDef, sDataSet* trainDS_);
	EXPORT ~sDataSet();

	EXPORT void build(int fromValStatus, int fromValSource);
	EXPORT void buildFromTS(int fromValSource, int fromValStatus);
	EXPORT void build_OUT(int fromValStatus, int fromValSource);
	EXPORT void unbuild(int fromValSource, int toValSource, int toValStatus);	//-- takes step 0 from predictionSBF, copy it into sourceTS->trsvalP
	EXPORT void reorder(int section, int FROMorderId, int TOorderId);

private:
	void mallocs1();
	void mallocs2();
	void frees();
	bool isSelected(int ts_f);
	void dumpPre(int valStatus, FILE** dumpFile);

	int BWfeaturesCnt=2;
	bool doDump;
	char* dumpPath;
};
