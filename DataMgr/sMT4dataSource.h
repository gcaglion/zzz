#pragma once
#include "../common.h"
#include "sDataSource.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../ConfigMgr/sCfg.h"
#include "FXData_enums.h"
#include "../MT4Data/sMT4Data.h"

struct sMT4DataSource : sDataSource {

	int sampleLen;
	char** bartime;
	numtype* sample;
	char basetime[DATE_FORMAT_LEN];
	numtype basebar[FXDATA_FEATURESCNT];
	char lastbartime[DATE_FORMAT_LEN];

	EXPORT sMT4DataSource(sObjParmsDef, int sampleLen_, long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, long iBaseBarT, double iBaseBarO, double iBaseBarH, double iBaseBarL, double iBaseBarC, double iBaseBarV);
	EXPORT sMT4DataSource(sObjParmsDef, sMT4Data* mt4db_);
	EXPORT sMT4DataSource(sCfgObjParmsDef);
	EXPORT ~sMT4DataSource();

	void load(char* pDate0, int pRecCount, char** oBarTime, numtype* oBarData, char* oBaseTime, numtype* oBaseBar, numtype* oBarWidth);

};
