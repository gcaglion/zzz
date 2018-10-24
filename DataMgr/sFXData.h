#pragma once
#include "../common.h"
#include "sDataSource.h"
#include "FXData_enums.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../OraData/sOraData.h"

#define FX_SYMBOL_MAXLEN XMLKEY_PARM_VAL_MAXLEN
#define FX_TIMEFRAME_MAXLEN XMLKEY_PARM_VAL_MAXLEN

struct sFXData : sDataSource {
	sOraData* db;
//	char* Symbol = new char[FX_SYMBOL_MAXLEN];
//	char* TimeFrame = new char[FX_TIMEFRAME_MAXLEN];
	char Symbol[XMLKEY_PARM_VAL_MAXLEN];
	char TimeFrame[XMLKEY_PARM_VAL_MAXLEN];
	bool IsFilled;

	EXPORT sFXData(sCfgObjParmsDef, sOraData* db_, char* symbol_, char* tf_, bool isFilled_, bool autoOpen);
	EXPORT sFXData(sCfgObjParmsDef, bool autoOpen);
	EXPORT ~sFXData();
	
	EXPORT void open();
	EXPORT void load(char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar);

};

