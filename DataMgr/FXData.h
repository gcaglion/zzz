#pragma once
#include "../common.h"
#include "DataSource.h"
#include "FXData_enums.h"
#include "../ConfigMgr/sCfg.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../OraOci/OraOci.h"

#define FX_SYMBOL_MAXLEN XMLKEY_PARM_VAL_MAXLEN
#define FX_TIMEFRAME_MAXLEN XMLKEY_PARM_VAL_MAXLEN
#define FXDataFeaturesCnt	5	// Fixed, OHLCV

struct sFXData : sDataSource {
	sOraConnection* db;
	char* Symbol = new char[FX_SYMBOL_MAXLEN];
	char* TimeFrame = new char[FX_TIMEFRAME_MAXLEN];
	Bool IsFilled;

	EXPORT sFXData(sCfgObjParmsDef, sOraConnection* db_, char* symbol_, char* tf_, Bool isFilled_);
	EXPORT sFXData(sCfgObjParmsDef);
	EXPORT ~sFXData();
	
	void load(char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar);


};

