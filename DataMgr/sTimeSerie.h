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
	numtype* val;
	numtype* trval;
	numtype* trsval;

	//-- these are of size [featuresCnt]
	char bdtime[DATE_FORMAT_LEN];
	numtype* base;
	numtype* dmin;
	numtype* dmax;
	numtype* scaleM;
	numtype* scaleP;

	//-- 
	bool doDump;

	//--
	EXPORT sTimeSerie(sObjParmsDef, sDataSource* sourceData_, char* date0_, int stepsCnt_, int dt_, int tsfCnt_, int* tsf_);
	EXPORT sTimeSerie(sCfgObjParmsDef);
	EXPORT ~sTimeSerie();

	EXPORT void load(char* date0_=nullptr);
	EXPORT void transform(int dt_=-1);
	EXPORT void scale(float scaleMin_, float scaleMax_);
	EXPORT void dump();

private:
	bool hasTR, hasTRS;
	void mallocs();
	void frees();
	void setDataSource();
};

