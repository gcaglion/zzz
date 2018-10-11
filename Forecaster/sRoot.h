#pragma once

#include "Forecaster.h"

struct sRoot : sObj {

	sCfg* clientCfg;
	sLogger* clientPersistor;

	sCfg* forecasterCfg;
	sForecaster* forecaster;
	
	sRoot(int argc_=0, char* argv_[]=nullptr);
	~sRoot();

private:
	//-- variables
	char clientCfgFileFullName[MAX_PATH];
	char forecasterCfgFileFullName[MAX_PATH];
	//-- overrides are for forecasterCfg only
	int cfgOverrideCnt=0;
	char cfgOverrideName[XMLLINE_MAXCNT][XMLKEY_PARM_NAME_MAXLEN];
	char cfgOverrideValS[XMLKEY_PARM_MAXCNT][XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];
	//--


	//-- functions

	void execute(int what);		//-- main container with try/catch block
	void loadCfg();	//-- load XML configurations for client and forecaster
	void createForecaster();	//-- create main Forecaster object from forecasterCfg


	static numtype MyRndDbl(numtype min, numtype max);
	void CLoverride(int argc, char* argv[]);
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

