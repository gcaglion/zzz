#include "Root.h"

//-- MetaTrader4 Wrapper Functions
extern "C" __declspec(dllexport) bool MTcreateEnv(int parmsCnt, char* parmOverrideString) {

	//-- 
	//-- 1. create root object. root constructor does everything else
	sRoot* root=nullptr;
	try {
		root=new sRoot(paramCnt, paramOverride);	//-- always takes default debugger settings
		root->execute();
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}

	terminate(true, "");

}

extern "C" __declspec(dllexport) int  MTUpdateClientInfo(int paramCnt, char* paramOverride, char* pDBCtx, unsigned int pElapsedms) {

	tForecastParms fParms; if (initfParms(paramCnt, paramOverride, &fParms) <0) return -1;

	//-- set DBCtx
	sscanf(pDBCtx, "%p", &fParms.DebugParms.DebugDB->DBCtx);

	//-- call UpdateClientInfo() in MyLogger library
	return (
		UpdateClientInfo(&fParms.DebugParms, GetCurrentProcessId(), (double)pElapsedms)
		);

}
extern "C" __declspec(dllexport) int  MTOraConnect(int paramCnt, char* paramOverride, char* oCtxS) {

	tForecastParms fParms; if (initfParms(paramCnt, paramOverride, &fParms) <0) return -1;

	if (OraConnect(&fParms.DebugParms, fParms.DebugParms.DebugDB)!=0) return -1;

	sprintf(oCtxS, "%p", fParms.DebugParms.DebugDB->DBCtx);

	return 0;
}
extern "C" __declspec(dllexport) void MTOraDisconnect(int paramCnt, char* paramOverride, char* pLogDBCtxS, int pCommit) {
	void* ctx = nullptr;
	sscanf(pLogDBCtxS, "%p", &ctx);
	OraDisconnect(ctx, pCommit);
}
extern "C" __declspec(dllexport) void MTOraCommit(int paramCnt, char* paramOverride, char* pLogDBCtxS) {
	void* ctx = nullptr;
	sscanf(pLogDBCtxS, "%p", &ctx);
	OraCommit(ctx);
}
extern "C" __declspec(dllexport) int  MTSaveTradeInfo(int paramCnt, char* paramOverride, int pBarId, char* pDBCtx, char* pLastBarT, double pLastBarO, double pLastBarH, double pLastBarL, double pLastBarC, char* pFirstBarT, double pFirstBarO, double pFirstBarH, double pFirstBarL, double pFirstBarC, double pPrevFH, double pPrevFL, double pCurrBid, double pCurrAsk, double pCurrFH, double pCurrFL, int pTradeType, double pTradeSize, double pTradeTP, double pTradeSL) {
	char vFirstBarT[20+1];
	char vLastBarT[20+1];
	int ret = 0;

	tForecastParms fParms; if (initfParms(paramCnt, paramOverride, &fParms) <0) return -1;

	//-- set DBCtx
	sscanf(pDBCtx, "%p", &fParms.DebugParms.DebugDB->DBCtx);

	//-- convert Bars date format
	convertBarTime(pLastBarT, vLastBarT);
	convertBarTime(pFirstBarT, vFirstBarT);

	//-- call SaveTradeInfo in MyLogger library
	return(
		SaveTradeInfo(
			&fParms.DebugParms, GetCurrentProcessId(), pBarId,
			vLastBarT, pLastBarO, pLastBarH, pLastBarL, pLastBarC,
			vFirstBarT, pFirstBarO, pFirstBarH, pFirstBarL, pFirstBarC,
			pPrevFH, pPrevFL,
			pCurrBid, pCurrAsk,
			pCurrFH, pCurrFL,
			pTradeType, pTradeSize, pTradeTP, pTradeSL
		)
		);

}
extern "C" __declspec(dllexport) int  MTSaveClientInfo(int paramCnt, char* paramOverride, char* pDBCtx, char* pCurrentBar, int pDoTraining, int pDoRun) {
	char vCurrentBar[20+1];

	tForecastParms fParms; if (initfParms(paramCnt, paramOverride, &fParms) <0) return -1;

	//-- set DBCtx
	sscanf(pDBCtx, "%p", &fParms.DebugParms.DebugDB->DBCtx);

	//-- convert Bar date format
	convertBarTime(pCurrentBar, vCurrentBar);

	//-- call SaveClientInfo in MyLogger library
	return (SaveClientInfo(&fParms.DebugParms, GetCurrentProcessId(), "MetaTrader", 1, vCurrentBar, 0, pDoTraining, pDoRun));
}
extern "C" __declspec(dllexport) int  MTgetForecast(int paramCnt, char* paramOverride, char* pLogDBCtxS, int progId, double* pHistoryDataH, double pHistoryBaseValH, double* pHistoryDataL, double pHistoryBaseValL, double* pHistoryBW, double* pValidationDataH, double pValidationBaseValH, double* pValidationDataL, double pValidationBaseValL, double* pFutureDataH, double* pFutureDataL, double* pFutureBW, double* oPredictedDataH, double* oPredictedDataL) {
	int ret;
	//-- High and Low arrays are put together in double arrays, with High in 0, Low in 1. BarWidth is duplicated
	double* vHistoryData[2];
	double  vHistoryBaseVal[2];
	double* vHistoryBW[2];
	double* vValidationData[2];
	double  vValidationBaseVal[2];
	double* vFutureData[2];
	double* vFutureBW[2];
	double* vPredictedData[2];
	//--
	vHistoryData[0] = pHistoryDataH;				vHistoryData[1] = pHistoryDataL;
	vHistoryBaseVal[0] = pHistoryBaseValH;			vHistoryBaseVal[1] = pHistoryBaseValL;
	vHistoryBW[0] = pHistoryBW;						vHistoryBW[1] = pHistoryBW;
	vValidationData[0] = pValidationDataH;			vValidationData[1] = pValidationDataL;
	vValidationBaseVal[0] = pValidationBaseValH;	vValidationBaseVal[1] = pValidationBaseValL;
	vFutureData[0] = pFutureDataH;					vFutureData[1] = pFutureDataL;
	vFutureBW[0] = pFutureBW;						vFutureBW[1] = pFutureBW;
	vPredictedData[0] = oPredictedDataH;			vPredictedData[1] = oPredictedDataL;

	//-- 1. create root object. root constructor does everything else
	sRoot* root=nullptr;
	try {
		root=new sRoot(argc, argv);	//-- always takes default debugger settings
		root->execute();
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}

	terminate(true, "");



	//-- 1. Create ConfigMgr object, and pass overrides
	//-- 2. create Forecaster object from Configuration
	safespawn(false, mainForecaster, newsname("Main_Forecaster"), defaultdbg, clientCfg, "/Forecaster");
	// Forecasting Parameters initialization. 
	tForecastParms fParms;
	char** param = (char**)malloc(ARRAY_PARAMETER_MAX_LEN*sizeof(char*)); for (int i = 0; i<ARRAY_PARAMETER_MAX_LEN; i++) param[i] = (char*)malloc(256);

	//-- a. set overrides from full string in paramOverride parameter
	paramCnt = cslToArray(paramOverride, ' ', param);
	if (CLProcess(paramCnt, param, fParms.iniParms) <0) return -3;
	//-- b. process ini file
	if (ForecastParamLoader(&fParms) <0) return -2;

	//-- set DBCtx
	sscanf(pLogDBCtxS, "%p", &fParms.DebugParms.DebugDB->DBCtx);

	//DumpArrayD(fParms.DataParms.HistoryLen, vHistoryData[0], "c:/temp/hd0.txt");
	//DumpArrayD(fParms.DataParms.HistoryLen, vHistoryData[1], "c:/temp/hd1.txt");

	LogWrite(&fParms.DebugParms, LOG_INFO, "BEFORE: oPredictedDataH[0]=%f \t oPredictedDataL[0]=%f \t vPredictedData[0][0]=%f \t vPredictedData[1][0]=%f\n", 4, oPredictedDataH[0], oPredictedDataL[0], vPredictedData[0][0], vPredictedData[1][0]);
	ret= getForecast(paramCnt, param, fParms.DebugParms.DebugDB->DBCtx, progId, vHistoryData, vHistoryBaseVal, vHistoryBW, vValidationData, vValidationBaseVal, 0, vFutureData, vFutureBW, vPredictedData);
	LogWrite(&fParms.DebugParms, LOG_INFO, "AFTER : oPredictedDataH[0]=%f \t oPredictedDataL[0]=%f \t vPredictedData[0][0]=%f \t vPredictedData[1][0]=%f\n", 4, oPredictedDataH[0], oPredictedDataL[0], vPredictedData[0][0], vPredictedData[1][0]);

	for (int i = 0; i<ARRAY_PARAMETER_MAX_LEN; i++) free(param[i]);
	free(param);
	return ret;
}
//--



//-- constructors/destructor
sRoot::sRoot(int argc_, char* argv_[]) : sObj(nullptr, newsname("RootObj"), nullptr) {
	CLoverride(argc_, argv_);
}
sRoot::sRoot(int argc_, char* argvS_) : sObj(nullptr, newsname("RootObj"), nullptr) {
	CLoverride(argc_, argvS_);
}
sRoot::~sRoot() {}

//-- executor
void sRoot::execute() {
	try {
		//-- Do stuff

		//-- 1. load clientCfg main XML configuration
		safespawn(false, clientCfg, newsname("Root_Config"), dbg, cfgFileFullName);
		//-- 2. create main Forecaster object from clientCfg
		safespawn(false, mainForecaster, newsname("Main_Forecaster"), defaultdbg, clientCfg, "/Forecaster");

		//-- 3.	Load Training_Start[]
		//-- 4. Save Client Log (elapsedTime is 0)
		//-- 5. Prepare, Train, Run for each Training_Start
		for (int siml = 0; siml<fParms->SimulationLength; siml++) {
		}

	}
	catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}
}

//-- private functions
void sRoot::CLoverride(int argc, char* argvS) {
	//-- first, split single-line override into name-value pairs
	const int maxORcnt = XMLKEY_PARM_MAXCNT*XMLKEY_MAXCNT;
	const int maxORlen = XMLKEY_PARM_NAME_MAXLEN+XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT+2;
	char** orNameValPair = (char**)malloc(maxORcnt*sizeof(char*)); for (int i=0; i<maxORcnt; i++) orNameValPair[i] = (char*)malloc(maxORlen*sizeof(char));
	cfgOverrideCnt=cslToArray(argvS, ' ', orNameValPair);
	//-- then, call proper handler
	CLoverride(cfgOverrideCnt, orNameValPair);
}
void sRoot::CLoverride(int argc, char* argv[]) {
	char orName[XMLKEY_PARM_NAME_MAXLEN];
	char orValS[XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];

	//-- set default cfgFileName
	getFullFileName("../Client.xml", cfgFileFullName);

	cfgOverrideCnt=0;
	for (int p=1; p<argc; p++) {
		if (!getValuePair(argv[p], &orName[0], &orValS[0], '=')) fail("wrong parameter format in command line: %s", argv[p]);
		if (_stricmp(orName, "--cfgFile")==0) {
			if (!getFullFileName(orValS, cfgFileFullName)) fail("could not set cfgFileFullName from override parameter: %s", orValS);
		} else {
			strcpy_s(cfgOverrideName[cfgOverrideCnt], XMLKEY_PARM_NAME_MAXLEN, orName);
			strcpy_s(cfgOverrideValS[cfgOverrideCnt], XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT, orValS);
			cfgOverrideCnt++;
		}
	}
}
