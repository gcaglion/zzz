#pragma once
#include "../common.h"
#include "DataSource_enums.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../FileData/sFileData.h"
#include "../OraData/sOraData.h"
#include "../MT4Data/sMT4Data.h"

struct sDataSource : sCfgObj {
	
	int type;
	sOraData* oradb;
	sFileData* filedb;
	sMT4Data* MT4db;

	int featuresCnt;
	bool calcBW;
	int BWfeatureH;
	int BWfeatureL;

	EXPORT sDataSource(sObjParmsDef, sOraData* oradb_, int featuresCnt_, bool calcBW_, int BWfeatureH_, int BWfeatureL_);
	EXPORT sDataSource(sObjParmsDef, sFileData* filedb_, int featuresCnt_, bool calcBW_, int BWfeatureH_, int BWfeatureL_);
	EXPORT sDataSource(sObjParmsDef, sMT4Data* MT4db_, int featuresCnt_, bool calcBW_, int BWfeatureH_, int BWfeatureL_);
	EXPORT sDataSource(sCfgObjParmsDef);
	EXPORT virtual ~sDataSource();

	virtual void getStartDates(char* date0_, int datesCnt_, char** oStartDates_){}
	virtual void open() {}
	virtual void load(char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar) {}
};
