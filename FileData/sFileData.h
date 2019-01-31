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
	EXPORT void commit();
	//--
	EXPORT void findPid(int pid_, bool* found_);
	EXPORT void saveClientInfo(int pid, int simulationId, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doInfer, const char* clientXMLfile_, const char* shapeXMLfile_, const char* actionXMLfile_, const char* engineXMLfile_);
	//--
	EXPORT void saveMSE(int pid, int tid, int mseCnt, int* duration, numtype* mseT, numtype* mseV);
	EXPORT void saveRun(int pid, int tid, int npid, int ntid, numtype mseR, int runStepsCnt, int tsFeaturesCnt_, int selectedFeaturesCnt, int* selectedFeature, int predictionLen, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted, numtype* barWidth_);
	//--
	EXPORT void saveEngineInfo(int pid, int engineType, int coresCnt, int sampleLen_, int predictionLen_, int featuresCnt_, int* coreId, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType);
	EXPORT void loadEngineInfo(int pid, int* engineType, int* coresCnt, int* sampleLen_, int* predictionLen_, int* featuresCnt_, bool* saveToDB_, bool* saveToFile_, const char* dbconn_, int* coreId, int* coreType, int* coreThreadId, int* parentCoresCnt, int** parentCore, int** parentConnType);
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
	EXPORT void saveCoreLoggerParms(int pid_, int tid_, int readFrom, bool saveToDB, bool saveToFile, bool saveMSEFlag, bool saveRunFlag, bool saveInternalsFlag, bool saveImageFlag);
	EXPORT void loadCoreLoggerParms(int pid_, int tid_, int* readFrom, bool* saveToDB, bool* saveToFile, bool* saveMSEFlag, bool* saveRunFlag, bool* saveInternalsFlag, bool* saveImageFlag);
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
	EXPORT void saveCoreNNInternalsSCGD(int pid_, int tid_, int iterationsCnt_, numtype* delta_, numtype* mu_, numtype* alpha_, numtype* beta_, numtype* lambda_, numtype* lambdau_, numtype* Gtse_old_, numtype* Gtse_new_, numtype* comp_, numtype* pnorm_, numtype* rnorm_, numtype* dwnorm_);
	//--
	EXPORT void saveXMLconfig(int simulationId_, int pid_, int tid_, int fileId_, int parmsCnt_, char** parmDesc_, char** parmVal_);

	//--
	EXPORT void getStartDates(char* StartDate, int DatesCount, char*** oDate);


private:
	void mallocs();
	bool isOpen=false;
};