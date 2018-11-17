#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sDataSource.h"
#include "sFXDataSource.h"
#include "sGenericDataSource.h"
#include "sMT4dataSource.h"
#include "TimeSerie_enums.h"

#define MAX_DATA_FEATURES 128
#define MAX_TSF_CNT	32

//-- val Source
#define TARGET		0
#define PREDICTED	1
#define SAMPLE		2	//-- used only in sDataSet
//-- val Status
#define BASE	0
#define TR	1
#define TRS	2

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
	numtype*** val;	//-- [Source][Status][len]


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

	EXPORT void load(int valSource, int valStatus, char* date0_=nullptr);
	EXPORT void transform(int valSource, int dt_);
	EXPORT void untransform(int fromValSource, int toValSource, int selectedFeaturesCnt_, int* selectedFeature_);
	EXPORT void scale(int valSource, int valStatus, float scaleMin_, float scaleMax_);
	EXPORT void unscale(int valSource, float scaleMin_, float scaleMax_, int selectedFeaturesCnt_, int* selectedFeature_, int skipFirstN_);
	EXPORT void dump(int valSource, int valStatus);

private:
	void mallocs1();
	void mallocs2();
	void frees();
	void setDataSource();
};

