#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sFXDataSource.h"
#include "sGenericDataSource.h"
#include "sMT4DataSource.h"
#include "TimeSerie_enums.h"

#define MAX_TS_FEATURES 128

struct sTS : sCfgObj {

	int stepsCnt;
	int featuresCnt;	// total
	int dt;

	char**  timestamp;
	numtype* val;
	numtype* valTR;
	char* timestampB;
	numtype* valB;

	numtype* TRmin;
	numtype* TRmax;

	bool doDump;
	char dumpPath[MAX_PATH];

	EXPORT sTS(sCfgObjParmsDef);
	EXPORT ~sTS();

	EXPORT void untransform();
	EXPORT void dump();

private:
	void setDataSource(sDataSource** dataSrc_);
};