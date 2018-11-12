#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sTimeSerie.h"
#include "../Algebra/Algebra.h"


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
	//-- array of pointers to any of the above : 
	numtype* _data[3][2];	//-- [Source][ordering]

	EXPORT sDataSet(sObjParmsDef, sTimeSerie* sourceTS_, int sampleLen_, int predictionLen_, int batchSamplesCnt_, int selectedFeaturesCnt_, int* selectedFeature_, bool doDump=false, const char* dumpPath_=nullptr);
	EXPORT sDataSet(sCfgObjParmsDef, int sampleLen_, int predictionLen_);
	EXPORT sDataSet(sObjParmsDef, sDataSet* trainDS_);
	EXPORT ~sDataSet();

	EXPORT void build(int fromValStatus, int fromValSource);
	EXPORT void unbuild(int fromValSource, int toValSource, int toValStatus);	//-- takes step 0 from predictionSBF, copy it into sourceTS->trsvalP
	EXPORT void reorder(int section, int FROMorderId, int TOorderId);

private:
	void mallocs1();
	void mallocs2();
	void frees();
	bool isSelected(int ts_f);
	void dumpPre(int valStatus, FILE** dumpFile);

	bool doDump;
	char* dumpPath;
};
