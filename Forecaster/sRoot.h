#pragma once

#include "../EngineMgr/sEngine.h"
#include "../DataMgr/sTS2.h"
#include "../DataMgr/sFXDataSource.h"
#include "../DataMgr/sGenericDataSource.h"
#include "../Logger/sLogger.h"
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

	EXPORT void trainClient(int simulationId_, const char* clientXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr, int overridesCnt_=0, char** overridePname_=nullptr, char** overridePcnt=nullptr);
	EXPORT void inferClient(int simulationId_, const char* clientXMLfile_, const char* inferXMLfile_, int savedEnginePid_, NativeReportProgress* progressPtr, int overridesCnt_=0, char** overridePname_=nullptr, char** overridePcnt=nullptr);
	
	//-- MT4 stuff
	int MT4clientPid;
	int MT4accountId;
	int MT4enginePid;
	int MT4sessionId;
	sEngine* MT4engine;
	bool MT4doDump;
	char MT4clientXMLFile[MAX_PATH];
	sCfg* MT4clientCfg;
	sLogger* MT4clientLog;

	EXPORT void getSeriesInfo(int* oSeriesCnt_, char* oSymbolsCSL_, char* oTimeFramesCSL_, char* oFeaturesCSL_, bool* oChartTrade_);
	EXPORT void getForecast(int seqId_, int extraSteps_, \
		int iseriesCnt_, long long* ifeatureMask_, \
		long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, \
		double* iBarMACD, double* iBarCCI, double* iBarATR, double* iBarBOLLH, double* iBarBOLLM, double* iBarBOLLL, double* iBarDEMA, double* iBarMA, double* iBarMOM, \
		long* iBaseBarT, double* iBaseBarO, double* iBaseBarH, double* iBaseBarL, double* iBaseBarC, double* iBaseBarV, \
		double* iBaseBarMACD, double* iBaseBarCCI, double* iBaseBarATR, double* iBaseBarBOLLH, double* iBaseBarBOLLM, double* iBaseBarBOLLL, double* iBaseBarDEMA, double* iBaseBarMA, double* iBaseBarMOM, \
		int oseriesCnt_, long long* ofeatureMask_, \
		long* oBarT, double* oBarO, double* oBarH, double* oBarL, double* oBarC, double* oBarV, \
		long* oBaseBarT, double* oBaseBarO, double* oBaseBarH, double* oBaseBarL, double* oBaseBarC, double* oBaseBarV, \
		double* oForecastO, double* oForecastH, double* oForecastL, double* oForecastC, double* oForecastV \
	);
	EXPORT void getActualFuture(char* iSymbol_, char* iTF_, char* iDate0_, char* oDate1_, double* oBarO, double* oBarH, double* oBarL, double* oBarC, double* oBarV);
	EXPORT void setMT4env(int clientPid_, int accountId_, char* clientXMLFile_, int savedEnginePid_, bool doDump_);
	EXPORT void saveTradeInfo(int iPositionTicket, char* iPositionOpenTime, char* iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iLastForecastO, double iLastForecastH, double iLastForecastL, double iLastForecastC, double iLastForecastV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult, int iTPhit, int iSLhit);
	EXPORT void saveClientInfo(int sequenceId, double iPositionOpenTime);
	EXPORT void MT4createEngine(int* oSampleLen_, int* oPredictionLen_, int* oFeaturesCnt_, int* oBatchSize);
	EXPORT void MT4commit();

	//-- temp stuff
	EXPORT void kaz();
	EXPORT void kazEnc();
	
private:

	//-- variables
	char clientCfgFileFullName[MAX_PATH];
	char forecasterCfgFileFullName[MAX_PATH];
	//-- overrides are for forecasterCfg only
	int cfgOverrideCnt=0;
	char cfgOverrideName[XMLLINE_MAXCNT][XMLKEY_PARM_NAME_MAXLEN];
	char cfgOverrideValS[XMLKEY_PARM_MAXCNT][XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];
	//--
//	void datasetPrepare(sTS* ts_, sEngine* eng_, sDS*** ds_, int dsSampleLen_, int dsTargetLen_, int dsBatchSize_, bool dsDoDump_, char* dsDumpPath_, bool loadEngine_);

	//-- functions
	void getSafePid(sLogger* persistor, int* pid);

	//-- the following are used by both trainClient() and inferClient()
	char clientffname[MAX_PATH];
	char shapeffname[MAX_PATH];
	char trainffname[MAX_PATH];
	char inferffname[MAX_PATH];
	char engineffname[MAX_PATH];
	char endtimeS[TIMER_ELAPSED_FORMAT_LEN];

	void MTpreprocess(int seriesCnt_, long long* featureMask_, int sampleLen, int sampleBarsCnt, int targetBarsCnt, int** selFcnt, int*** selF, long* BarT, double* BarO, double* BarH, double* BarL, double* BarC, double* BarV, double* BarMACD, double* BarCCI, double* BarATR, double* BarBOLLH, double* BarBOLLM, double* BarBOLLL, double* BarDEMA, double* BarMA, double* BarMOM, long* BaseBarT, double* BaseBarO, double* BaseBarH, double* BaseBarL, double* BaseBarC, double* BaseBarV, double* BaseBarMACD, double* BaseBarCCI, double* BaseBarATR, double* BaseBarBOLLH, double* BaseBarBOLLM, double* BaseBarBOLLL, double* BaseBarDEMA, double* BaseBarMA, double* BaseBarMOM, char*** BarTimeS, char** BarBTimeS, numtype** Bar, numtype** BarB);
	
};

//-- client closure
#define terminate(success_, ...) { \
	int pid=root->pid; \
	delete root; \
	int ret; \
	if(success_){ \
		printf("Client success. ProcessId=%d\n", pid); \
		ret = 0; \
	} else { \
		printf("Client failed: %s\n", __VA_ARGS__); \
		printf("Press any key..."); getchar(); \
		ret = -1; \
	} \
	return ret; \
}

