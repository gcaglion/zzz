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

struct sOraDB : sCfgObj {

	EXPORT sOraDB(sObjParmsDef, const char* DBUserName_, const char* DBPassword_, const char* DBConnString_, bool autoOpen=false);
	EXPORT sOraDB(sCfgObjParmsDef, bool autoOpen=true);
	EXPORT ~sOraDB();

	EXPORT void getFlatOHLCV(char* pSymbol, char* pTF, char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar);
	EXPORT void SaveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV);
	EXPORT void SaveRun(int pid, int tid, int setid, int npid, int ntid, int runCnt, int featuresCnt, int* feature, numtype* prediction, numtype* actual);
	EXPORT void SaveW(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void LoadW(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void SaveClient(int pid, char* clientName, DWORD startTime, DWORD duration, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun);
	EXPORT void Commit();


private:
	void* env;
	void* conn;

	char* DBUserName = new char[DBUSERNAME_MAXLEN];
	char* DBPassword = new char[DBPASSWORD_MAXLEN];
	char* DBConnString = new char[DBCONNSTRING_MAXLEN];

	void open();
	void close();
	void commit();

};
