#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sFXDataSource.h"
#include "sGenericDataSource.h"
#include "sMT4DataSource.h"
#include "TimeSerie_enums.h"
#undef fail
#include "../Wavelib/wavelet2d.h"
#include "../BaseObj/sDbgMacros.h"

struct sTS2 :sCfgObj {
	int stepsCnt;
	int dataSourcesCnt;
	int* featuresCnt;
	int WTlevel;

	int dt;
	int WTtype;

	char** timestamp;	// [stepsCnt]
	numtype**** val;	// [stepsCnt][dataSourcesCnt][featuresCnt][WTlevel+2]
	numtype**** valTR;	// [stepsCnt][dataSourcesCnt][featuresCnt][WTlevel+2]
	numtype*** TRmin;	// [dataSourcesCnt][featuresCnt][WTlevel+2]
	numtype*** TRmax;	// [dataSourcesCnt][featuresCnt][WTlevel+2]
	char* timestampB;
	numtype*** valB;	// [dataSourcesCnt][featuresCnt][WTlevel+2]

	bool doDump;
	char dumpPath[MAX_PATH];

	EXPORT sTS2(sCfgObjParmsDef);
	EXPORT ~sTS2();
	EXPORT void dump();

private:
	void dumpToFile(FILE* file, numtype**** val_);
	void mallocs1();
	void setDataSource(sDataSource** dataSrc_);
	void WTcalc(int d, int f, numtype* dsvalSF);
	void transform(int d, int f, int l);
};