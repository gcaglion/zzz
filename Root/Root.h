#pragma once
#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfg.h"
#include "../Forecaster/Forecaster.h"

struct sRoot : sObj {

	sCfg* clientCfg;
	sForecaster* mainForecaster;

	sRoot(int argc_=0, char* argv_[]=nullptr);
	sRoot(int argc_=0, char* argvS_=nullptr);
	~sRoot();

	void execute();

private:
	//-- variables
	char cfgFileFullName[MAX_PATH];
	int cfgOverrideCnt;
	char cfgOverrideName[XMLLINE_MAXCNT][XMLKEY_PARM_NAME_MAXLEN];
	char cfgOverrideValS[XMLKEY_PARM_MAXCNT][XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];
	//-- functions
	void CLoverride(int argc, char* argv[]);
	void CLoverride(int argc, char* argvS);
};


//-- MetaTrader4 Wrapper Functions
extern "C" __declspec(dllexport) int  MTUpdateClientInfo(int paramCnt, char* paramOverride, char* pDBCtx, unsigned int pElapsedms);
extern "C" __declspec(dllexport) int  MTOraConnect(int paramCnt, char* paramOverride, char* oCtxS);
extern "C" __declspec(dllexport) void MTOraDisconnect(int paramCnt, char* paramOverride, char* pLogDBCtxS, int pCommit);
extern "C" __declspec(dllexport) void MTOraCommit(int paramCnt, char* paramOverride, char* pLogDBCtxS);
extern "C" __declspec(dllexport) int  MTSaveTradeInfo(int paramCnt, char* paramOverride, int pBarId, char* pDBCtx, char* pLastBarT, double pLastBarO, double pLastBarH, double pLastBarL, double pLastBarC, char* pFirstBarT, double pFirstBarO, double pFirstBarH, double pFirstBarL, double pFirstBarC, double pPrevFH, double pPrevFL, double pCurrBid, double pCurrAsk, double pCurrFH, double pCurrFL, int pTradeType, double pTradeSize, double pTradeTP, double pTradeSL);
extern "C" __declspec(dllexport) int  MTSaveClientInfo(int paramCnt, char* paramOverride, char* pDBCtx, char* pCurrentBar, int pDoTraining, int pDoRun);
extern "C" __declspec(dllexport) int  MTgetForecast(int paramCnt, char* paramOverride, char* pLogDBCtxS, int progId, double* pHistoryDataH, double pHistoryBaseValH, double* pHistoryDataL, double pHistoryBaseValL, double* pHistoryBW, double* pValidationDataH, double pValidationBaseValH, double* pValidationDataL, double pValidationBaseValL, double* pFutureDataH, double* pFutureDataL, double* pFutureBW, double* oPredictedDataH, double* oPredictedDataL);
//--

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
