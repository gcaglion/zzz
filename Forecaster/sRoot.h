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

struct sRoot : sCfgObj {

	int pid;

	EXPORT sRoot(NativeReportProgress* progressReporter);
	EXPORT ~sRoot();

	EXPORT void trainClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr=nullptr);
	EXPORT void  bothClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr=nullptr);
	EXPORT void inferClient(int simulationId_, const char* clientXMLfile_, const char* inferXMLfile_, int savedEnginePid_, NativeReportProgress* progressPtr=nullptr);
	
	//-- MT4 stuff
	int MT4accountId;
	int MT4enginePid;
	sEngine* MT4engine;
	int MT4sampleLen;
	int MT4predictionLen;
	bool MT4useVolume;
	int MT4dt;
	bool MT4doDump;
	char MT4clientXMLFile[MAX_PATH];
	EXPORT void getForecast(int* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, double iBaseBarO, double iBaseBarH, double iBaseBarL, double iBaseBarC, double iBaseBarV, double* oForecastH, double* oForecastL);
	EXPORT void setMT4env(int accountId_, char* clientXMLFile_, int savedEnginePid_, bool useVolume_, int dt_, bool doDump_);
	EXPORT void MT4createEngine();

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
	void getSafePid(sLogger* persistor, int* pid);

	//-- the following are used by both trainClient() and inferClient()
	char clientffname[MAX_PATH];
	char shapeffname[MAX_PATH];
	char trainffname[MAX_PATH];
	char inferffname[MAX_PATH];
	char engineffname[MAX_PATH];
	char endtimeS[TIMER_ELAPSED_FORMAT_LEN];
	sCfg* clientCfg; sCfg* shapeCfg; sCfg* trainCfg; sCfg* inferCfg; sCfg* engCfg;
	sDataShape* shape;
	sDataSet* trainDS; sLogger* trainLog;
	sDataSet* inferDS; sLogger* inferLog;
	sEngine* engine;
	sLogger* clientLog;


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

