#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sDataSource.h"
#include "sFXDataSource.h"
#include "sGenericDataSource.h"
#include "sMT4dataSource.h"
#include "TimeSerie_enums.h"

#define MAX_DATA_FEATURES 128
#define MAX_TSF_CNT	32

//-- val Status
#define BASE	0
#define TR	1
#define TRS	2
//-- val Source
#define TARGET		0
#define PREDICTED	1

struct sTimeSerie : sCfgObj {

	sDataSource* sourceData;

	char* date0;
	int stepsCnt;
	int len;

	//-- transformation and statistical features
	int dt;
	int tsfCnt;
	int* tsf;

	//-- these are of size [len]
	char** dtime;
	//-- 'A' stands for Actual
	numtype* valA;
	numtype* trvalA;
	numtype* trsvalA;
	//-- 'P' stands for Predicted
	numtype* valP;
	numtype* trvalP;
	numtype* trsvalP;
	//-- this is just a pointer 
	numtype* val[3][2];	//-- [Status][Source]

	//-- these are of size [featuresCnt]
	char bdtime[DATE_FORMAT_LEN];
	numtype* base;
	numtype* dmin;
	numtype* dmax;
	numtype* scaleM;
	numtype* scaleP;

	//-- 
	bool doDump;
	char* dumpPath;

	//--
	EXPORT sTimeSerie(sObjParmsDef, sDataSource* sourceData_, const char* date0_, int stepsCnt_, int dt_, int tsfCnt_, int* tsf_, const char* dumpPath_=nullptr);
	EXPORT sTimeSerie(sCfgObjParmsDef);
	EXPORT ~sTimeSerie();

	EXPORT void load(char* date0_=nullptr);
	EXPORT void transform(int dt_=-1);
	EXPORT void untransform(int selectedFeaturesCnt_, int* selectedFeature_, numtype* fromData_, numtype* toData_);
	EXPORT void scale(float scaleMin_, float scaleMax_);
	EXPORT void unscale(float scaleMin_, float scaleMax_, int selectedFeaturesCnt_, int* selectedFeature_, int sampleLen_, int valSource);
	EXPORT void dump(int status, int source);

private:
	void mallocs1();
	void mallocs2();
	void frees();
	void setDataSource();
};

