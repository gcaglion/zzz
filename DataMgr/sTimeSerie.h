#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sDataSource.h"
#include "sFXDataSource.h"
#include "sGenericDataSource.h"
#include "sMT4dataSource.h"
#include "TimeSerie_enums.h"

#define MAX_DATA_FEATURES 128
#define MAX_TSF_CNT	32

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
	EXPORT void untransform(numtype* fromData_, numtype* toData_);
	EXPORT void scale(float scaleMin_, float scaleMax_);
	EXPORT void unscale(float scaleMin_, float scaleMax_, int selectedFeaturesCnt_, int* selectedFeatures_, numtype* fromData_, numtype* toData_);
	EXPORT void dump(bool prediction_ = false);

private:
	bool hasTR, hasTRS;
	void mallocs1();
	void mallocs2();
	void frees();
	void setDataSource();
};

