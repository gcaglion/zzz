#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "TimeSerie_enums.h"
#include "sDataSource.h"
#include "sFXDataSource.h"
#include "sGenericDataSource.h"
#include "sMT4DataSource.h"

#define MAX_DATA_FEATURES 128
#define MAX_TSF_CNT	32

typedef struct sTimeSerie : sCfgObj {

	//-- data source
	sDataSource* sourceData=nullptr;

	char date0[DATE_FORMAT_LEN];
	int steps;
	int featuresCnt;
	int len;
	int dt;	// data transformation
	bool BWcalc;	// Bar width calc
	int tsfCnt;
	int* tsf;

	// data scaling: boundaries depend on core the samples are fed to, M/P are different for each feature
	numtype scaleMin, scaleMax;
	numtype *scaleM, *scaleP;
	numtype *dmin, *dmax;

	numtype* d;		//-- host   data ( steps X featuresCnt )
	char** dtime;	//-- may always be useful...
	numtype* bd;	//-- host   base data ( 1 X featuresCnt )
	char* bdtime;	
	bool hasTR=false;
	numtype* d_tr;
	bool hasTRS=false;
	numtype* d_trs;

	//-- constructors / destructor
	EXPORT void sTimeSeriecommon();

	EXPORT sTimeSerie(sCfgObjParmsDef);
	EXPORT ~sTimeSerie();
	
	EXPORT void load(char* date0_);
	EXPORT void transform(int dt_);
	EXPORT void scale(numtype scaleMin_, numtype scaleMax_);

	EXPORT void TrS(int dt_, numtype scaleMin_, numtype scaleMax_);
	EXPORT void unTrS(numtype scaleMin_, numtype scaleMax_);

	EXPORT void dump();

private:
	void setDataSource(sCfg* cfg);
	bool doDump;

} tTimeSerie;

