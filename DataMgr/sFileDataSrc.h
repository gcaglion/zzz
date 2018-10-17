#pragma once
#include "../common.h"
#include "sDataSource.h"
#include "FileData_enums.h"
#include "../ConfigMgr/sCfgObj.h"

struct sFileDataSrc : sDataSource {
	FILE* srcFile;
	int fieldSep;
	int featuresCnt;

	EXPORT sFileDataSrc(sCfgObjParmsDef, FILE* srcFile_, int fieldSep_=COMMA_SEPARATOR, bool calcBW_=false, int BWfeatureH_=1, int BWfeatureL_=2, bool autoOpen=true);
	EXPORT sFileDataSrc(sCfgObjParmsDef, bool autoOpen);
	EXPORT ~sFileDataSrc();

	EXPORT void open();
	EXPORT void load(const char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar);

private:
	void getFeaturesCnt(int* oFeaturesCnt_);

};
