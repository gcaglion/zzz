#pragma once

#include "../DataMgr/sDataSet.h"
#include "../DataMgr/sDataShape.h"
#include "../DataMgr/sFXDataSource.h"
#include "../DataMgr/sGenericDataSource.h"
#include "../DataMgr/sMT4DataSource.h"
#include "../Logger/sLogger.h"
#include "../EngineMgr/sEngine.h"
#include "../Utils/Utils.h"

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

	EXPORT void trainClient(int simulationId_, const char* clientXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr);
	EXPORT void inferClient(int simulationId_, const char* clientXMLfile_, const char* inferXMLfile_, int savedEnginePid_, NativeReportProgress* progressPtr);
	
	//-- MT4 stuff
	int MT4clientPid;
	int MT4accountId;
	int MT4enginePid;
	int MT4sessionId;
	sEngine* MT4engine;
	int MT4dt;
	bool MT4doDump;
	char MT4clientXMLFile[MAX_PATH];
	sCfg* MT4clientCfg;
	sLogger* MT4clientLog;

	EXPORT void getSeriesInfo(int* oSeriesCnt_, char* oSymbolsCSL_, char* oTimeFramesCSL_, char* oFeaturesCSL_, bool* oChartTrade_);
	EXPORT void getForecast(int seriesCnt_, int dt_, int* featureMask_, long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, long* iBaseBarT, double* iBaseBarO, double* iBaseBarH, double* iBaseBarL, double* iBaseBarC, double* iBaseBarV, double* oForecastO, double* oForecastH, double* oForecastL, double* oForecastC, double* oForecastV);
	EXPORT void setMT4env(int clientPid_, int accountId_, char* clientXMLFile_, int savedEnginePid_, int dt_, bool doDump_);
	EXPORT void saveTradeInfo(int iPositionTicket, char* iPositionOpenTime, char* iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult);
	EXPORT void MT4createEngine(int* oSampleLen_, int* oPredictionLen_, int* oFeaturesCnt_);

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
	void getSafePid(sLogger* persistor, int* pid);

	//-- the following are used by both trainClient() and inferClient()
	char clientffname[MAX_PATH];
	char shapeffname[MAX_PATH];
	char trainffname[MAX_PATH];
	char inferffname[MAX_PATH];
	char engineffname[MAX_PATH];
	char endtimeS[TIMER_ELAPSED_FORMAT_LEN];
	sCfg* clientCfg; sCfg* trainCfg; sCfg* inferCfg; sCfg* engCfg;
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

