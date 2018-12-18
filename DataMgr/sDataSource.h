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
	sMT4Data* mt4db;

	int featuresCnt;
	bool calcBW;
	int BWfeatureH;
	int BWfeatureL;

	sDataSource(sObjParmsDef, int featuresCnt_, bool calcBW_, int BWfeatureH_, int BWfeatureL_);
	sDataSource(sCfgObjParmsDef);
	virtual ~sDataSource();

	virtual void getStartDates(char* date0_, int datesCnt_, char*** oStartDates_); 
	virtual void open();
	virtual void load(char* pDate0, int pRecCount, char** oBarTime, numtype* oBarData, char* oBaseTime, numtype* oBaseBar, numtype* oBarWidth)=0;

};
