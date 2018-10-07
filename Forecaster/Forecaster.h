#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfg.h"
#include "../DataMgr/sData.h"
#include "../EngineMgr/Engine.h"
#include "../Logger/Logger.h"

struct sForecaster : sCfgObj {
	
	sData* data;
	sEngine* engine;
	sLogger* persistor;

	EXPORT sForecaster(sCfgObjParmsDef, sData* data_, sEngine* engine_, sLogger* persistor_);
	EXPORT sForecaster(sCfgObjParmsDef);
	EXPORT ~sForecaster();

};

//-- MetaTrader4 Wrapper Functions
extern "C" __declspec(dllexport) int  MTUpdateClientInfo(	int paramCnt, char* paramOverride, char* pDBCtx, unsigned int pElapsedms);
extern "C" __declspec(dllexport) int  MTOraConnect(			int paramCnt, char* paramOverride, char* oCtxS);
extern "C" __declspec(dllexport) void MTOraDisconnect(		int paramCnt, char* paramOverride, char* pLogDBCtxS, int pCommit);
extern "C" __declspec(dllexport) void MTOraCommit(			int paramCnt, char* paramOverride, char* pLogDBCtxS);
extern "C" __declspec(dllexport) int  MTSaveTradeInfo(		int paramCnt, char* paramOverride, int pBarId, char* pDBCtx, char* pLastBarT, double pLastBarO, double pLastBarH, double pLastBarL, double pLastBarC, char* pFirstBarT, double pFirstBarO, double pFirstBarH, double pFirstBarL, double pFirstBarC, double pPrevFH, double pPrevFL, double pCurrBid, double pCurrAsk, double pCurrFH, double pCurrFL, int pTradeType, double pTradeSize, double pTradeTP, double pTradeSL);
extern "C" __declspec(dllexport) int  MTSaveClientInfo(		int paramCnt, char* paramOverride, char* pDBCtx, char* pCurrentBar, int pDoTraining, int pDoRun);
extern "C" __declspec(dllexport) int  MTgetForecast(		int paramCnt, char* paramOverride, char* pLogDBCtxS, int progId, double* pHistoryDataH, double pHistoryBaseValH, double* pHistoryDataL, double pHistoryBaseValL, double* pHistoryBW, double* pValidationDataH, double pValidationBaseValH, double* pValidationDataL, double pValidationBaseValL, double* pFutureDataH, double* pFutureDataL, double* pFutureBW, double* oPredictedDataH, double* oPredictedDataL);
//--
