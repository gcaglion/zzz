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

	EXPORT sOraData(sObjParmsDef, const char* DBUserName_, const char* DBPassword_, const char* DBConnString_);
	EXPORT sOraData(sCfgObjParmsDef);
	EXPORT ~sOraData();
	//--
	EXPORT void getStartDates(char* symbol_, char* timeframe_, bool isFilled_, char* StartDate, int DatesCount, char*** oDate);
	EXPORT void getFlatOHLCV2(char* pSymbol, char* pTF, char* date0_, int stepsCnt, char** oBarTime, numtype* oBarData, char* oBarTime0, numtype* oBaseBar, numtype* oBarWidth);
	//--
	EXPORT void open();
	EXPORT void close();
	EXPORT void commit();
	//--
	EXPORT void saveClientInfo(int pid, int simulationId, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doInfer, const char* clientXMLfile_, const char* shapeXMLfile_, const char* actionXMLfile_, const char* engineXMLfile_);
	//--
	EXPORT void saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV);
	EXPORT void saveRun(int pid, int tid, int npid, int ntid, int runStepsCnt, int tsFeaturesCnt_, int selectedFeaturesCnt, int* selectedFeature, int predictionLen, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted, numtype* barWidth);
	//--
	EXPORT void saveEngineInfo(int pid, int engineType, int coresCnt, int* coreId, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType);
	EXPORT void loadEngineInfo(int pid, int* engineType, int* coresCnt, int* coreId, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType);
	EXPORT int getSavedEnginePids(int maxPids_, int* oPid);
	//--
	EXPORT void saveCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveCoreGAImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveCoreSOMImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveCoreSVMImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveCoreDUMBImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
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
	EXPORT void loadCoreNNparms(int pid, int tid, char** levelRatioS_, char** levelActivationS_, bool* useContext_, bool* useBias_, int* maxEpochs_, numtype* targetMSE_, int* netSaveFrequency_, bool* stopOnDivergence_, int* BPalgo_, float* learningRate_, float* learningMomentum_);
	EXPORT void loadCoreGAparms(int pid, int tid, int* p1, numtype* p2);
	EXPORT void loadCoreSVMparms(int pid, int tid, int* p1, numtype* p2);
	EXPORT void loadCoreSOMparms(int pid, int tid, int* p1, numtype* p2);
	EXPORT void loadCoreDUMBparms(int pid, int tid, int* p1, numtype* p2);
	//--
	EXPORT void saveCoreNNInternalsSCGD(int pid_, int tid_, int iterationsCnt_, numtype* delta_, numtype* mu_, numtype* alpha_, numtype* beta_, numtype* lambda_, numtype* lambdau_, numtype* comp_, numtype* Gtse_old_, numtype* Gtse_new_, numtype* pnorm_, numtype* rnorm_, numtype* dwnorm_);

private:
	void* env = nullptr;
	void* conn = nullptr;
	void* stmt = nullptr;
	void* rset = nullptr;
	bool isOpen = false;

	char* DBUserName = new char[DBUSERNAME_MAXLEN];
	char* DBPassword = new char[DBPASSWORD_MAXLEN];
	char* DBConnString = new char[DBCONNSTRING_MAXLEN];

	char sqlS[SQL_MAXLEN];

	//-- generic dml
	void sqlExec(char* sqlS);
	//-- generic select. each returns a single array for a single data field
	void sqlGet(int len, int** valP, const char* sqlMask, ...);
	void sqlGet(int len, numtype** valP, const char* sqlMask, ...);
	void sqlGet(int len, char*** valP, const char* sqlMask, ...);
	void sqlGet(int len, bool** valP, const char* sqlMask, ...);
};
