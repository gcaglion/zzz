#pragma once

#include "Timer.h"
#include "Forecaster.h"

#include "../DataMgr/sFXDataSource.h"
#include "../DataMgr/sGenericDataSource.h"
#include "../DataMgr/sMT4DataSource.h"

struct sRoot : sObj {

	int pid;

	sForecaster* forecaster;
	
	EXPORT sRoot(int argc_=0, char* argv_[]=nullptr);
	EXPORT ~sRoot();

	EXPORT void newClient();
	EXPORT void trainClient(const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_);
	EXPORT void inferClient(const char* clientXMLfile_, const char* shapeXMLfile_, const char* inferXMLfile_, int savedEnginePid_);

private:

	sTimer timer;

	//-- variables
	char clientCfgFileFullName[MAX_PATH];
	char forecasterCfgFileFullName[MAX_PATH];
	//-- overrides are for forecasterCfg only
	int cfgOverrideCnt=0;
	char cfgOverrideName[XMLLINE_MAXCNT][XMLKEY_PARM_NAME_MAXLEN];
	char cfgOverrideValS[XMLKEY_PARM_MAXCNT][XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];
	//--
	void CLoverride(int argc, char* argv[]);

	//-- functions
	void mallocSimulationDates(sCfg* clientCfg, int* simLen, char*** simTrainStart, char*** simInferStart, char*** simValidStart);
	void getStartDates(sDataSet* ds, char* date00_, int len, char*** oDates);
	void getSimulationDates(sCfg* clientCfg_, int* simLen, char** simTrainStart, char** simInferStart, char** simValidStart);

};

//-- MetaTrader calls
EXPORT bool MTcreateForecasterEnv(char* baseConfigFileFullName, int overridesCnt, char* overridesFullString, char* oRootObjPointerString);
EXPORT bool MTgetForecast(int paramOverrideCnt, char** paramOverride, void* LogDBCtx, int pTestId, double** pHistoryData, double* pHistoryBaseVal, double** pHistoryBW, double** pValidationData, double* pValidationBaseVal, int haveActualFuture, double** pFutureData, double** pFutureBW, double** oPredictedData);


//-- client closure
#define terminate(success_, ...) { \
	delete root; \
	int ret; \
	if(success_){ \
		printf("Client success. ProcessId=%d\n", GetCurrentProcessId()); \
		ret = 0; \
	} else { \
		printf("Client failed: %s\n", __VA_ARGS__); \
		ret = -1; \
	} \
	printf("Press any key..."); getchar(); \
	return ret; \
}

