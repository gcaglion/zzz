#pragma once
#include "../common.h"
#include "sDataSource.h"
#include "../FileData/sFileData.h"
#include "FileData_enums.h"
#include "../ConfigMgr/sCfgObj.h"

struct sGenericDataSource : sDataSource {

	int fieldSep;
	int featuresCnt;

	EXPORT sGenericDataSource(sObjParmsDef, sFileData* file_, int fieldSep_=COMMA_SEPARATOR, bool calcBW_=false, int BWfeatureH_=1, int BWfeatureL_=2);
	EXPORT sGenericDataSource(sObjParmsDef, sOraData* db_, int fieldSep_=COMMA_SEPARATOR, bool calcBW_=false, int BWfeatureH_=1, int BWfeatureL_=2);
	EXPORT sGenericDataSource(sCfgObjParmsDef);
	EXPORT ~sGenericDataSource();

	EXPORT void open();
	EXPORT void load(const char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar);

private:
	void getFeaturesCnt(int* oFeaturesCnt_);

};
