#pragma once
#include "../common.h"
#include "sDataSource.h"
#include "FileData_enums.h"
#include "../ConfigMgr/sCfgObj.h"

struct sFileData : sDataSource {
	FILE* srcFile;
	int fieldSep;
	int featuresCnt;

	EXPORT sFileData(sCfgObjParmsDef, FILE* srcFile_, int fieldSep_=COMMA_SEPARATOR, bool calcBW_=false, int BWfeatureH_=1, int BWfeatureL_=2, bool autoOpen=true);
	EXPORT sFileData(sCfgObjParmsDef, bool autoOpen);
	EXPORT ~sFileData();

	EXPORT void open();
	EXPORT void load(const char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar);

private:
	void getFeaturesCnt(int* oFeaturesCnt_);

};
