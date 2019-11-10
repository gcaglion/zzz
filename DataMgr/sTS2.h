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
	int* dataSourcesCnt;	// IN/OUT
	int** featuresCnt;
	int* WTlevel;	// IN/OUT
	int* WTtype;	// IN/OUT

	int dt;

	char** timestamp;	// [stepsCnt]
	numtype***** val;	// [stepsCnt][IN/OUT][dataSourcesCnt][featuresCnt][WTlevel+2]
	numtype***** valTR;	// [stepsCnt][IN/OUT][dataSourcesCnt][featuresCnt][WTlevel+2]
	numtype***** valTRS;// [stepsCnt][IN/OUT][dataSourcesCnt][featuresCnt][WTlevel+2]
	numtype***** prd;	// [stepsCnt][IN/OUT][dataSourcesCnt][featuresCnt][WTlevel+2]
	numtype***** prdTR;	// [stepsCnt][IN/OUT][dataSourcesCnt][featuresCnt][WTlevel+2]
	numtype***** prdTRS;// [stepsCnt][IN/OUT][dataSourcesCnt][featuresCnt][WTlevel+2]
	numtype**** TRmin;	// [IN/OUT][dataSourcesCnt][featuresCnt][WTlevel+2]
	numtype**** TRmax;	// [IN/OUT][dataSourcesCnt][featuresCnt][WTlevel+2]
	numtype**** scaleM;	// [IN/OUT][dataSourcesCnt][featuresCnt][WTlevel+2]
	numtype**** scaleP;	// [IN/OUT][dataSourcesCnt][featuresCnt][WTlevel+2]
	char* timestampB;
	numtype**** valB;	// [IN/OUT][dataSourcesCnt][featuresCnt][WTlevel+2]

	bool doDump;
	char dumpPath[MAX_PATH];

	EXPORT sTS2(sCfgObjParmsDef);
	EXPORT ~sTS2();
	EXPORT void dump(bool predicted=false);
	EXPORT void scale(float scaleMin_, float scaleMax_);
	EXPORT void unscale();
	EXPORT void untransform();
	EXPORT void getDataSet(int sampleLen_, int targetLen_, int* oSamplesCnt, numtype** oSample, numtype** oTarget);

private:
	void dumpToFile(FILE* file, int i, numtype***** val_);
	void mallocs1();
	void setDataSource(sDataSource** dataSrc_);
	void WTcalc(int i, int d, int f, numtype* dsvalSF);
	void transform(int i, int d, int f, int l);
};