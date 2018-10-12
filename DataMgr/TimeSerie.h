#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "TimeSerie_enums.h"
#include "FXData.h"
#include "FileData.h"
#include "MT4Data.h"

#define MAX_DATA_FEATURES 128
#define MAX_TSF_CNT	32

typedef struct sTimeSerie : sCfgObj {

	//-- data source
	sDataSource* sourceData=nullptr;

	char* date0 = new char[XMLKEY_PARM_VAL_MAXLEN];
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
	EXPORT void sTimeSeriecommon(int steps_, int featuresCnt_, int tsfCnt_=0, int* tsf_=nullptr);

	EXPORT sTimeSerie(sCfgObjParmsDef, int steps_, int featuresCnt_);
	EXPORT sTimeSerie(sCfgObjParmsDef, sFXData* dataSource_, int steps_, char* date0_, int dt_);
	EXPORT sTimeSerie(sCfgObjParmsDef, sFileData* dataSource_, int featuresCnt_, int steps_, char* date0_, int dt_);
	EXPORT sTimeSerie(sCfgObjParmsDef);
	EXPORT ~sTimeSerie();
	
	EXPORT void transform(int dt_);
	EXPORT void scale(numtype scaleMin_, numtype scaleMax_);

	EXPORT void TrS(int dt_, numtype scaleMin_, numtype scaleMax_);
	EXPORT void unTrS(numtype scaleMin_, numtype scaleMax_);

	EXPORT void dump(char* dumpFileName="C:/temp/TSdump.csv");

private:
	bool setDataSource(sCfg* cfg);

} tTimeSerie;

