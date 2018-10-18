#pragma once
#include "../common.h"
#include "../BaseObj/sObj.h"
#include "DataSource_enums.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../ConfigMgr/sCfg.h"
#include "../FileData/sFileData.h"
#include "../OraData/sOraData.h"

struct sDataSource : sCfgObj {
	
	int type;
	sOraData* db;
	sFileData* file;

	int featuresCnt;
	bool calcBW;
	int BWfeatureH;
	int BWfeatureL;

	sDataSource(sCfgObjParmsDef, int type_, int featuresCnt_, bool calcBW_, int BWfeatureH_, int BWfeatureL_);
	sDataSource(sCfgObjParmsDef);
	~sDataSource();

	virtual void getStartDates(char* date0_, int datesCnt_, char** oStartDates_){}
	virtual void open() {}
	virtual void load(char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar) {}
};
