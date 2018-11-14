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
	EXPORT void getStartDates(char* StartDate, int DatesCount, char*** oDate);

	EXPORT void saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV);
	EXPORT void saveRun(int pid, int tid, int npid, int ntid, int runStepsCnt, int tsFeaturesCnt_, int selectedFeaturesCnt, int* selectedFeature, int predictionLen, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted);
	EXPORT void saveClientInfo(int pid, int simulationId, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doTrainRun, bool doTestRun);
	EXPORT void commit();
	//--
	EXPORT void saveCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadEngineInfo(int pid, int* engineType, int* coresCnt, int* coreId, int* coreType, int* parentCoresCnt, int** parentCore, int** parentConnType);
	//--
	EXPORT void saveCoreNNparms(int pid, int tid, char* levelRatioS_, char* levelActivationS_, bool useContext_, bool useBias_, int maxEpochs_, numtype targetMSE_, int netSaveFrequency_, bool stopOnDivergence_, int BPalgo_, float learningRate_, float learningMomentum_);
	EXPORT void loadCoreNNparms(int pid, int tid, char** levelRatioS_, char** levelActivationS_, bool* useContext_, bool* useBias_, int* maxEpochs_, numtype* targetMSE_, int* netSaveFrequency_, bool* stopOnDivergence_, int* BPalgo_, float* learningRate_, float* learningMomentum_);

private:
	void mallocs();
	bool isOpen=false;
};