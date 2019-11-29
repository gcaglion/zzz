#pragma once

#include "../ConfigMgr/sCfgObj.h"
#include "sFXDataSource.h"
#include "sGenericDataSource.h"
#include "TimeSerie_enums.h"
#undef fail
#include "../Wavelib/wavelet2d.h"
#include "../BaseObj/sDbgMacros.h"

#define MAX_TS_FEATURES 128

struct sTS2 :sCfgObj {
	int stepsCnt;
	int* dataSourcesCnt;	// IN/OUT
	int** featuresCnt;
	int*** dt;	// [IN/OUT][dataSourcesCnt][featuresCnt]
	int* WTlevel;	// IN/OUT
	int* WTtype;	// IN/OUT

	int sampleLen;
	int targetLen;
	int inputCnt;
	int outputCnt;
	int batchSize;
	int samplesCnt=0;
	int IOshift;
	numtype* sample;
	numtype* target;
	numtype* prediction;
	numtype* sampleTRS;
	numtype* targetTRS;
	numtype* predictionTRS;
	numtype* sampleBSF;
	numtype* targetBSF;
	numtype* predictionBSF;

	char*** timestamp;	// [stepsCnt][IN/OUT]
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
	char** timestampB;	// [IN/OUT]
	numtype**** valB;	// [IN/OUT][dataSourcesCnt][featuresCnt][WTlevel+2]

	bool doDump;
	char dumpPath[MAX_PATH];

	EXPORT sTS2(sCfgObjParmsDef);
	EXPORT sTS2(sObjParmsDef, \
		int ioShift_, int stepsCnt_, int*** dt_, int sampleLen_, int targetLen_, int batchSize_, bool doDump_, \
		char*** INtimestamp_, char** INtimestampB_, \
		int INdataSourcesCnt_, int* INfeaturesCnt_, int INWTtype_, int INWTlevel_, numtype* INval_, numtype* INvalB_, \
		char*** OUTtimestamp_, char** OUTtimestampB_, \
		int OUTdataSourcesCnt_, int* OUTfeaturesCnt_, int OUTWTtype_, int OUTWTlevel_, numtype* OUTval_, numtype* OUTvalB_\
	);
	EXPORT ~sTS2();
	EXPORT void dump();
	EXPORT void dumpDS();
	EXPORT void scale(float scaleMin_, float scaleMax_);
	EXPORT void unscale();
	EXPORT void untransform();
	EXPORT void buildDataSet();
	EXPORT void getPrediction();
	EXPORT void invert();
	EXPORT void invertDS();
	EXPORT void slide(int steps_);
	EXPORT void slideDS(int steps_);

private:
	void dumpToFile(FILE* file, int i, bool predicted, numtype***** val_);
	void _dumpDS(FILE* file, numtype* prs, numtype* prt, numtype* prp);
	void mallocs1();
	void setDataSource(sDataSource** dataSrc_);
	void WTcalc(int i, int d, int f, numtype* dsvalSF);
	void transform(int i, int d, int f, int l);
	
};