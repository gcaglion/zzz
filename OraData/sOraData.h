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
	EXPORT void open();
	EXPORT void close();
	EXPORT void commit();
	//--
	EXPORT void getStartDates(char* symbol_, char* timeframe_, bool isFilled_, char* StartDate, int DatesCount, char*** oDate);
	EXPORT void getFlatOHLCV2(char* pSymbol, char* pTF, char* date0_, int stepsCnt, char** oBarTime, float* oBarData, char* oBarTime0, float* oBaseBar);
	//--
	EXPORT void saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV);
	EXPORT void saveRun(int pid, int tid, int npid, int ntid, int runStepsCnt, int tsFeaturesCnt_, int selectedFeaturesCnt, int* selectedFeature, int predictionLen, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted);
	//--
	EXPORT void saveCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveClientInfo(int pid, int simulationId, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doTrainRun, bool doTestRun);
	//--
	EXPORT void saveEngineInfo(int pid, int engineType, int coresCnt, int* coreId, int* coreType, int* parentCoresCnt, int** parentCore, int** parentConnType);
	EXPORT void loadEngineInfo(int pid, int* engineType, int* coresCnt, int* coreId, int* coreType, int* parentCoresCnt, int** parentCore, int** parentConnType);
	//--
	EXPORT void saveCoreNNparms(int pid, int tid, char* levelRatioS_, char* levelActivationS_, bool useContext_, bool useBias_, int maxEpochs_, numtype targetMSE_, int netSaveFrequency_, bool stopOnDivergence_, int BPalgo_, float learningRate_, float learningMomentum_);

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
