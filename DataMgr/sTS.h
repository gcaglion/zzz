#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sFXDataSource.h"
#include "sGenericDataSource.h"
#include "sMT4DataSource.h"

#define MAX_TS_FEATURES 128

struct sTS : sCfgObj {

	int stepsCnt;
	int featuresCnt;	// total
	int dt;

	char**  timestamp;
	numtype* val;
	char* timestampB;
	numtype* valB;

	bool doDump;
	char dumpPath[MAX_PATH];

	EXPORT sTS(sCfgObjParmsDef);
	EXPORT ~sTS();

	void setDataSource(sDataSource** dataSrc_);
	EXPORT void dump();
};