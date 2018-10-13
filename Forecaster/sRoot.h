#pragma once

#include "Forecaster.h"

struct sRoot : sObj {

	sCfg* testerCfg;
	sLogger* testerPersistor;

	sCfg* forecasterCfg;
	sForecaster* forecaster;
	
	EXPORT sRoot(int argc_=0, char* argv_[]=nullptr);
	EXPORT ~sRoot();

	EXPORT void tester();

private:
	//-- variables
	char testerCfgFileFullName[MAX_PATH];
	char forecasterCfgFileFullName[MAX_PATH];
	//-- overrides are for forecasterCfg only
	int cfgOverrideCnt=0;
	char cfgOverrideName[XMLLINE_MAXCNT][XMLKEY_PARM_NAME_MAXLEN];
	char cfgOverrideValS[XMLKEY_PARM_MAXCNT][XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];

	//-- functions
	void getStartDates(sDataSet* ds, int len, char** oDates);
	void CLoverride(int argc, char* argv[]);
	//--
	static numtype MyRndDbl(numtype min, numtype max);
	void testDML();

};

//-- MetaTrader calls
EXPORT bool MTcreateForecasterEnv(char* baseConfigFileFullName, int overridesCnt, char* overridesFullString, char* oRootObjPointerString);


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

