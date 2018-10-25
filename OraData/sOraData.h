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

	EXPORT sOraData(sObjParmsDef, const char* DBUserName_, const char* DBPassword_, const char* DBConnString_, bool autoOpen);
	EXPORT sOraData(sCfgObjParmsDef, bool autoOpen);
	EXPORT ~sOraData();
	//--
	EXPORT void open();
	EXPORT void close();
	EXPORT void commit();
	//--
	EXPORT void getStartDates(char* symbol_, char* timeframe_, bool isFilled_, char* StartDate, int DatesCount, char** oDate);
	EXPORT void getFlatOHLCV(char* pSymbol, char* pTF, char* pDate0, int pastStepsCnt, char** oBarTimeH, float* oBarDataH, int futureStepsCnt, char** oBarTimeF, float* oBarDataF, char* oBarTime0, float* oBaseBar);
	EXPORT void getFlatOHLCV2(char* pSymbol, char* pTF, char* date0_, int stepsCnt, char** oBarTime, float* oBarData, char* oBarTime0, float* oBaseBar);
	//--
	EXPORT void saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV);
	EXPORT void saveRun(int pid, int tid, int npid, int ntid, int barsCnt, int featuresCnt, int* feature, numtype* actual, numtype* predicted);
	EXPORT void saveW(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadW(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveClient(int pid, char* clientName, DWORD startTime, DWORD duration, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun);

private:
	void* env = nullptr;
	void* conn = nullptr;

	char* DBUserName = new char[DBUSERNAME_MAXLEN];
	char* DBPassword = new char[DBPASSWORD_MAXLEN];
	char* DBConnString = new char[DBCONNSTRING_MAXLEN];


};
