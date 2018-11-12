#pragma once
#include "../common.h"
#include "sDataSource.h"
#include "FXData_enums.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../OraData/sOraData.h"
#include "../FileData/sFileData.h"

#define FX_SYMBOL_MAXLEN XMLKEY_PARM_VAL_MAXLEN
#define FX_TIMEFRAME_MAXLEN XMLKEY_PARM_VAL_MAXLEN

struct sFXDataSource : sDataSource {

	char* Symbol = new char[FX_SYMBOL_MAXLEN];
	char* TimeFrame = new char[FX_TIMEFRAME_MAXLEN];
	bool IsFilled;

	EXPORT sFXDataSource(sObjParmsDef, sOraData* db_, const char* symbol_, const char* tf_, bool isFilled_);
	EXPORT sFXDataSource(sObjParmsDef, sFileData* file_, const char* symbol_, const char* tf_, bool isFilled_);
	EXPORT sFXDataSource(sCfgObjParmsDef);
	EXPORT ~sFXDataSource();
	
	EXPORT void getStartDates(string date0_, int datesCnt_, string* oStartDates_);
	EXPORT void open();
	EXPORT void load(char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar);

};

