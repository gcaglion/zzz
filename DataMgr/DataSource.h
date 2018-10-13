#pragma once
#include "../common.h"
#include "../BaseObj/sObj.h"
#include "DataSource_enums.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../ConfigMgr/sCfg.h"

struct sDataSource : sCfgObj {
	
	int type;
	int featuresCnt;
	bool calcBW;
	int BWfeatureH;
	int BWfeatureL;

	sDataSource(sCfgObjParmsDef, int type_, int featuresCnt_, bool calcBW_, int BWfeatureH_, int BWfeatureL_);
	~sDataSource();

	void load(char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar) {}
	void open(){}
};
