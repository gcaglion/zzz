#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sFXDataSource.h"
#include "sGenericDataSource.h"
#include "sMT4DataSource.h"
#include "TimeSerie_enums.h"
#undef fail
#include "../Wavelib/wavelet2d.h"
#include "../BaseObj/sDbgMacros.h"

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

	numtype** lfa;	//-- [feature]
	numtype*** hfd;	//-- [feature][decomplevel]
	numtype** valFFT;

	numtype* TRmin;
	numtype* TRmax;

	bool doDump;
	char dumpPath[MAX_PATH];

	EXPORT sTS(sObjParmsDef, int stepsCnt_, int featuresCnt_, int dt_, char** timestamp_, numtype* val_, char* timestampB_, numtype* valB_, bool doDump_, char* dumpPath_=nullptr);
	EXPORT sTS(sCfgObjParmsDef);
	EXPORT ~sTS();

	EXPORT void untransform();
	EXPORT void dump();
	EXPORT void FFTcalc(int decompLevel_, int waveletType_);

private:
	void setDataSource(sDataSource** dataSrc_);
	void transform();
	void mallocs1();
};