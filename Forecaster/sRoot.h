#pragma once

#include "Timer.h"
#include "Forecaster.h"

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

struct sFork {

	virtual bool getStatus()=0;

private:
	bool isSet=false;
	bool status;

};

struct sRoot : sObj {

	int pid;

	sTimer* timer;
	sLogger* clientPersistor;
	sCfg* clientCfg;
	sCfg* forecasterCfg;
	sForecaster* forecaster;
	sEngine* engine;
	
	
	EXPORT sRoot(int argc_=0, char* argv_[]=nullptr);
	EXPORT ~sRoot();

	EXPORT void tester();
	
	void maintree();
	void kaz4();

private:
	//-- variables
	char clientCfgFileFullName[MAX_PATH];
	char forecasterCfgFileFullName[MAX_PATH];
	//-- overrides are for forecasterCfg only
	int cfgOverrideCnt=0;
	char cfgOverrideName[XMLLINE_MAXCNT][XMLKEY_PARM_NAME_MAXLEN];
	char cfgOverrideValS[XMLKEY_PARM_MAXCNT][XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];

	//-- functions
	void getStartDates(sDataSet* ds, char* date00_, int len, char*** oDates);
	void CLoverride(int argc, char* argv[]);
	//--
	static numtype MyRndDbl(numtype min, numtype max);
	void testDML();
	//--
	void block1();
	void block2();
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

