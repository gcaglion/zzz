#pragma once

#include "../DataMgr/sDataSet.h"
#include "../DataMgr/sDataShape.h"
#include "../DataMgr/sFXDataSource.h"
#include "../DataMgr/sGenericDataSource.h"
#include "../DataMgr/sMT4DataSource.h"
#include "../Logger/sLogger.h"
#include "../EngineMgr/sEngine.h"

class sdp {
public:
	int p1;
	float p2;
	char msg[DBG_MSG_MAXLEN];
};

typedef void(__stdcall *NativeReportProgress) (int, void*);

struct sRoot : sCfgObj {

	int pid;

	EXPORT sRoot(int argc_=0, char* argv_[]=nullptr);
	EXPORT ~sRoot();

	EXPORT void trainClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr=nullptr);
	EXPORT void  bothClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr=nullptr);
	EXPORT void inferClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* inferXMLfile_, const char* engineXMLfile_, int savedEnginePid_, NativeReportProgress* progressPtr=nullptr);
	
	//-- temp stuff
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

