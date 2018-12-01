#pragma once

/*#include <msclr\marshal.h>
using namespace msclr::interop;
using namespace System;
*/

#include "../DataMgr/sDataSet.h"
#include "../DataMgr/sDataShape.h"
#include "../DataMgr/sFXDataSource.h"
#include "../DataMgr/sGenericDataSource.h"
#include "../DataMgr/sMT4DataSource.h"
#include "../Logger/sLogger.h"
#include "../EngineMgr/sEngine.h"

struct sRoot : sCfgObj {

	int pid;

	EXPORT sRoot(int argc_=0, char* argv_[]=nullptr);
	EXPORT ~sRoot();

	EXPORT void trainClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_);
	EXPORT void  bothClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_);
	EXPORT void inferClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* inferXMLfile_, const char* engineXMLfile_, int savedEnginePid_);
	
	//-- temp stuff
	EXPORT void trainClient(int simulationId_, const char* cfgPath_);
	EXPORT void inferClient(int simulationId_, const char* cfgPath_, int savedEnginePid_);
	EXPORT void bothClient(int simulationId_, const char* cfgPath_);
	EXPORT void kaz();

private:

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

//-- client closure
#define terminate(success_, ...) { \
	delete root; \
	int ret; \
	if(success_){ \
		printf("Client success. ProcessId=%d\n", GetCurrentProcessId()); \
		ret = 0; \
	} else { \
		printf("Client failed: %s\n", __VA_ARGS__); \
		printf("Press any key..."); getchar(); \
		ret = -1; \
	} \
	return ret; \
}

extern "C" __declspec(dllexport) int _bothClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_);
