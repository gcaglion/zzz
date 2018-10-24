#pragma once

#include "Forecaster.h"
#include "sClientLogger.h"

#include "../DataMgr/sFXDataSource.h"
#include "../DataMgr/sGenericDataSource.h"
#include "../DataMgr/sMT4DataSource.h"

struct sss : sObj {
	sss(sObjParmsDef) : sObj(sObjParmsVal) {
		fail("Failure");
	}
};
struct ss : sObj {
	sss* sss1;
	ss(sObjParmsDef) : sObj(sObjParmsVal) {
		sss1=new sss(this, newsname("sss1"), defaultdbg);
	}
};
struct s : sObj {
	ss* ss1;
	s(sObjParmsDef) : sObj(sObjParmsVal) {
		ss1=new ss(this, newsname("ss1"), defaultdbg);
	}
};

struct sRoot : sObj {

	sCfg* testerCfg;
	sCfg* forecasterCfg;
	sForecaster* forecaster;
	
	EXPORT sRoot(int argc_=0, char* argv_[]=nullptr);
	EXPORT ~sRoot();

	EXPORT void tester();
	EXPORT void kaz();
	EXPORT void kaz2();
	EXPORT void kaz3();

private:
	//-- variables
	char testerCfgFileFullName[MAX_PATH];
	char forecasterCfgFileFullName[MAX_PATH];
	//-- overrides are for forecasterCfg only
	int cfgOverrideCnt=0;
	char cfgOverrideName[XMLLINE_MAXCNT][XMLKEY_PARM_NAME_MAXLEN];
	char cfgOverrideValS[XMLKEY_PARM_MAXCNT][XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];

	//-- functions
	void getStartDates(sDataSet* ds, char* date00_, int len, char** oDates);
	void CLoverride(int argc, char* argv[]);
	//--
	static numtype MyRndDbl(numtype min, numtype max);
	void testDML();

};

//-- MetaTrader calls
EXPORT bool MTcreateForecasterEnv(char* baseConfigFileFullName, int overridesCnt, char* overridesFullString, char* oRootObjPointerString);
EXPORT bool MTgetForecast(int paramOverrideCnt, char** paramOverride, void* LogDBCtx, int pTestId, double** pHistoryData, double* pHistoryBaseVal, double** pHistoryBW, double** pValidationData, double* pValidationBaseVal, int haveActualFuture, double** pFutureData, double** pFutureBW, double** oPredictedData);


//-- client closure
#define terminate(success_, ...) { \
	delete root; \
	int ret; \
	if(success_){ \
		printf("Client success. \n"); \
		ret = 0; \
	} else { \
		printf("Client failed: %s\n", __VA_ARGS__); \
		ret = -1; \
	} \
	printf("Press any key..."); getchar(); \
	return ret; \
}

