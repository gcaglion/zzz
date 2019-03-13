#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sDataShape.h"
#include "sTimeSerie.h"
#include "../Algebra/Algebra.h"

#define DATASET_MAX_SOURCETS_CNT 16

//-- ordering Ids
#define SBF	0
#define BFS	1

struct sDataSet : sCfgObj {

	sDataShape* shape;
	int sourceTScnt;
	sTimeSerie** sourceTS;
	int* selectedTSfeaturesCnt;
	int** selectedTSfeature;

	int samplesCnt;
	int batchSamplesCnt;
	int batchCnt;
	bool hasTargets;

	//-- sample, target, prediction are stored in  order (Sample-Bar-Feature)
	numtype* sampleSBF;		
	numtype* targetSBF;		
	numtype* predictionSBF;	
	//-- network training requires BFS ordering
	numtype* sampleBFS;		
	numtype* targetBFS;		
	numtype* predictionBFS;	

	EXPORT sDataSet(sObjParmsDef, int sourceTScnt_, sTimeSerie** sourceTS_, int* selectedTSfeaturesCnt_, int** selectedTSfeature_, int sampleLen_, int predictionLen_, int batchSamplesCnt_, bool doDump_=false, char* dumpPath_=nullptr);
	EXPORT sDataSet(sCfgObjParmsDef);
	EXPORT ~sDataSet();

	EXPORT void load(int fromValSource, int fromValStatus);
	EXPORT void build(int fromValSource, int fromValStatus);
	EXPORT void unbuild(int fromValSource, int toValSource, int toValStatus);	//-- takes step 0 from predictionSBF, copy it into sourceTS->trsvalP

	EXPORT void setBFS();
	EXPORT void setSBF();
	EXPORT void SBF2BFS(int batchId, int barCnt, numtype* fromSBF, numtype* toBFS);
	EXPORT void BFS2SBF(int batchId, int barCnt, numtype* fromBFS, numtype* toSBF);


private:
	void mallocs1();
	void setSamples();
	void mallocs2();
	void frees();
	void dumpPre(int valStatus, FILE** dumpFile);

	bool doDump;
	char* dumpPath;
};
