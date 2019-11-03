#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfgObj.h"

//-- limits
#define DBUSERNAME_MAXLEN XMLKEY_PARM_VAL_MAXLEN
#define DBPASSWORD_MAXLEN XMLKEY_PARM_VAL_MAXLEN
#define DBCONNSTRING_MAXLEN XMLKEY_PARM_VAL_MAXLEN

#define SQL_MAXLEN	4096
#define MAX_INSERT_BATCH_COUNT 500

struct sOraData : sCfgObj {

	char* DBUserName = new char[DBUSERNAME_MAXLEN];
	char* DBPassword = new char[DBPASSWORD_MAXLEN];
	char* DBConnString = new char[DBCONNSTRING_MAXLEN];

	EXPORT sOraData(sObjParmsDef, const char* DBUserName_, const char* DBPassword_, const char* DBConnString_);
	EXPORT sOraData(sCfgObjParmsDef);
	EXPORT ~sOraData();
	//--
	EXPORT void getFutureBar(char* iSymbol_, char* iTF_, char* iDate0_, char* oDate1_, double* oBarO, double* oBarH, double* oBarL, double* oBarC, double* oBarV);
	EXPORT void getStartDates(char* symbol_, char* timeframe_, bool isFilled_, char* StartDate, int DatesCount, char*** oDate);
	EXPORT void getFlatOHLCV2(char* pSymbol, char* pTF, const char* date0_, int stepsCnt, char** oBarTime, numtype* oBarData, char* oBarTime0, numtype* oBaseBar, numtype* oBarWidth);
	//--
	EXPORT void open();
	EXPORT void close();
	EXPORT void commit();
	//--
	EXPORT void findPid(int pid_, bool* found_);
	EXPORT void saveClientInfo(int pid, int sequenceId, int simulationId, int npid, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doInfer, const char* clientXMLfile_, const char* shapeXMLfile_, const char* actionXMLfile_, const char* engineXMLfile_);
	//--
	EXPORT void saveMSE(int pid, int tid, int mseCnt, int* duration, numtype* mseT, numtype* mseV);
//	EXPORT void saveRun(int pid, int tid, int npid, int ntid, numtype mseR, int runStepsCnt, int featuresCnt_, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actualBASE, numtype* predictedBASE);
	EXPORT void saveRun(int pid, int tid, int npid, int ntid, int seqId, numtype mseR, int runStepsCnt, char** posLabel, int featuresCnt_, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actualBASE, numtype* predictedBASE);
	//--
	EXPORT void saveEngineInfo(int pid, int engineType, int coresCnt, int sampleLen_, int predictionLen_, int featuresCnt_, int batchSize_, int WNNdecompLevel_, int WNNwaveletType_, bool saveToDB_, bool saveToFile_, sOraData* dbconn_, int* coreId, int* coreLayer, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType, numtype* trMin_, numtype* trMax_, numtype** fftMin_, numtype** fftMax_);
	EXPORT void loadEngineInfo(int pid, int* engineType_, int* coresCnt, int* sampleLen_, int* predictionLen_, int* featuresCnt_, int* batchSize_, int* WNNdecompLevel_, int* WNNwaveletType_, bool* saveToDB_, bool* saveToFile_, sOraData* dbconn_, int* coreId, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType, numtype* trMin_, numtype* trMax_, numtype** fftMin_, numtype** fftMax_);
	EXPORT int getSavedEnginePids(int maxPids_, int* oPid);
	//--
	EXPORT void saveCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W, int Fcnt, numtype* F);
	EXPORT void saveCoreGAImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveCoreSOMImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveCoreSVMImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveCoreDUMBImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W, int Fcnt, numtype* F);
	EXPORT void loadCoreGAImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadCoreSOMImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadCoreSVMImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadCoreDUMBImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	//--
	EXPORT void saveCoreNNparms(int pid, int tid, char* levelRatioS_, char* levelActivationS_, bool useContext_, bool useBias_, int maxEpochs_, numtype targetMSE_, int netSaveFrequency_, bool stopOnDivergence_, int BPalgo_, float learningRate_, float learningMomentum_, int SCGDmaxK_);
	EXPORT void saveCoreGAparms(int pid, int tid, int p1, numtype p2);
	EXPORT void saveCoreSVMparms(int pid, int tid, int p1, numtype p2);
	EXPORT void saveCoreSOMparms(int pid, int tid, int p1, numtype p2);
	EXPORT void saveCoreDUMBparms(int pid, int tid, int p1, numtype p2);
	EXPORT void loadCoreNNparms(int pid, int tid, char** levelRatioS_, char** levelActivationS_, bool* useContext_, bool* useBias_, int* maxEpochs_, float* targetMSE_, int* netSaveFrequency_, bool* stopOnDivergence_, int* BPalgo_, float* learningRate_, float* learningMomentum_);
	EXPORT void loadCoreGAparms(int pid, int tid, int* p1, numtype* p2);
	EXPORT void loadCoreSVMparms(int pid, int tid, int* p1, numtype* p2);
	EXPORT void loadCoreSOMparms(int pid, int tid, int* p1, numtype* p2);
	EXPORT void loadCoreDUMBparms(int pid, int tid, int* p1, numtype* p2);
	//--
	EXPORT void saveCoreLoggerParms(int pid_, int tid_, int readFrom, bool saveToDB, bool saveToFile, bool saveMSEFlag, bool saveRunFlag, bool saveInternalsFlag, bool saveImageFlag);
	EXPORT void loadCoreLoggerParms(int pid_, int tid_, int* readFrom, bool* saveToDB, bool* saveToFile, bool* saveMSEFlag, bool* saveRunFlag, bool* saveInternalsFlag, bool* saveImageFlag);
	//--
	EXPORT void saveCoreNNInternalsSCGD(int pid_, int tid_, int iterationsCnt_, numtype* delta_, numtype* mu_, numtype* alpha_, numtype* beta_, numtype* lambda_, numtype* lambdau_, numtype* comp_, numtype* Gtse_old_, numtype* Gtse_new_, numtype* pnorm_, numtype* rnorm_, numtype* dwnorm_);
	//--
	EXPORT void loadDBConnInfo(int pid_, int tid_, char** oDBusername, char** oDBpassword, char** oDBconnstring);
	EXPORT void saveDBConnInfo(int pid_, int tid_, char* oDBusername, char* oDBpassword, char* oDBconnstring);
	//--
	EXPORT void saveTradeInfo(int MT4clientPid, int MT4sessionId, int MT4accountId, int MT4enginePid, int iPositionTicket, char* iPositionOpenTime, char* iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iLastForecastO, double iLastForecastH, double iLastForecastL, double iLastForecastC, double iLastForecastV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult, int iTPhit, int iSLhit);
	//--
	EXPORT void saveXMLconfig(int simulationId_, int pid_, int tid_, int fileId_, int parmsCnt_, char** parmDesc_, char** parmVal_);

private:
	void* env = nullptr;
	void* conn = nullptr;
	void* stmt = nullptr;
	void* rset = nullptr;
	bool isOpen = false;

	char sqlS[SQL_MAXLEN];

	//-- generic dml
	void sqlExec(char* sqlS);
	//-- generic select. each returns a single array for a single data field
	void sqlGet(int len, int** valP, const char* sqlMask, ...);
	void sqlGet(int len, numtype** valP, const char* sqlMask, ...);
	void sqlGet(int len, char*** valP, const char* sqlMask, ...);
	void sqlGet(int len, bool** valP, const char* sqlMask, ...);
};
