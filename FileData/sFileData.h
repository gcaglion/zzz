#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"

#define FILE_MODE_READ		0
#define FILE_MODE_WRITE		1
#define FILE_MODE_APPEND	2

struct sFileData : sCfgObj {

	int filesCnt;
	char** fileFullName;	// one file for each log (Client, MSE, Run, Internals, Image)
	FILE** fileH;

	EXPORT sFileData(sObjParmsDef, int openMode_, bool autoOpen_, int filesCnt_, char** fileFullName_);
	EXPORT sFileData(sObjParmsDef, int openMode_, bool autoOpen_);
	EXPORT sFileData(sCfgObjParmsDef);
	EXPORT ~sFileData();

	EXPORT void open(int mode_);
	EXPORT void close();
	EXPORT void getStartDates(char* startDate_, int datesCnt_, char** oDate_);

	EXPORT void saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV);
	EXPORT void saveRun(int pid, int tid, int npid, int ntid, int barsCnt, int tsFeaturesCnt_, int selectedFeaturesCnt, int* selectedFeature, int predictionLen, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted);
	EXPORT void saveW(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadW(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveClientInfo(int pid, const char* clientName, double startTime, double elapsedSecs, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun);
	EXPORT void commit();


private:
	void mallocs();
	bool isOpen=false;
};