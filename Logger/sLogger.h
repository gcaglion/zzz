#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../FileData/sFileData.h" 
#include "../OraData/sOraData.h"

struct sLogger : sCfgObj {

	//-- How many logs
	int logsCnt;

	
	//-- Read sources. only one can be used
	const int OraData  = 0;
	const int FileData = 1;
	int source;

	//-- Write destinations. can be more than 1 simultaneously
	bool saveToDB;
	bool saveToFile;

	//-- databases. for both read and write
	sOraData* oradb;
	sFileData* filedb;

	EXPORT sLogger(sObjParmsDef, int readFrom_, bool saveToDB_, bool saveToFile_);	//-- this is called by one of sCoreLogger constructors
	EXPORT sLogger(sObjParmsDef, sOraData* oradb_);
	EXPORT sLogger(sObjParmsDef, sFileData* filedb_);
	EXPORT sLogger(sObjParmsDef, int readFrom_, sOraData* oradb_, sFileData* filedb_);
	EXPORT sLogger(sCfgObjParmsDef);
	EXPORT ~sLogger();

	EXPORT virtual void open();
	EXPORT virtual void close();
	EXPORT virtual void commit();
	//--
	EXPORT void findPid(int pid_, bool* found_);
	EXPORT void saveClientInfo(int pid, int sequenceId, int simulationId, int npid, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doInfer, const char* clientXMLfile_, const char* shapeXMLfile_, const char* actionXMLfile_, const char* engineXMLfile_);
	//--
	EXPORT void saveMSE(int pid, int tid, int mseCnt, int* duration, numtype* mseT, numtype* mseV);
	EXPORT void saveRun(int pid, int tid, int npid, int ntid, int seqId, numtype mseR, int runStepsCnt, char** posLabel, int featuresCnt_, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actualBASE, numtype* predictedBASE);
	//--
	EXPORT void saveEngineInfo(int pid, int engineType, int coresCnt, int sampleLen_, int predictionLen_, int featuresCnt_, int batchSize_, int WNNdecompLevel_, int WNNwaveletType_, bool saveToDB_, bool saveToFile_, sOraData* dbconn_, int* coreId, int* coreLayer, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType, numtype* trMin_, numtype* trMax_, numtype** fftMin_, numtype** fftMax_);
	EXPORT void loadEngineInfo(int pid, int* engineType_, int* coresCnt, int* sampleLen_, int* predictionLen_, int* featuresCnt_, int* batchSize_, int* WNNdecompLevel_, int* WNNwaveletType_, bool* saveToDB_, bool* saveToFile_, sOraData* dbconn_, int* coreId, int* coreType, int* tid, int* parentCoresCnt, int** parentCore, int** parentConnType, numtype* trMin_, numtype* trMax_, numtype** fftMin_, numtype** fftMax_);
	//--
	EXPORT void saveCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W, int Fcnt, numtype* F);
	EXPORT void saveCoreGAImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveCoreSOMImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveCoreSVMImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void saveCoreDUMBImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	//--
	EXPORT void loadCoreNNImage(int pid, int tid, int epoch, int Wcnt, numtype* W, int Fcnt, numtype* F);
	EXPORT void loadCoreGAImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadCoreSOMImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadCoreSVMImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	EXPORT void loadCoreDUMBImage(int pid, int tid, int epoch, int Wcnt, numtype* W);
	//--
	EXPORT void saveCoreNNparms(int pid, int tid, char* levelRatioS_, char* levelActivationS_, bool useContext_, bool useBias_, int maxEpochs_, numtype targetMSE_, int netSaveFrequency_, bool stopOnDivergence_, int BPalgo_, float learningRate_, float learningMomentum_, int SCGDmaxK_);
	EXPORT void saveCoreGAparms(int pid, int tid, int p1, int p2);
	EXPORT void saveCoreSOMparms(int pid, int tid, int p1, int p2);
	EXPORT void saveCoreSVMparms(int pid, int tid, int p1, int p2);
	EXPORT void saveCoreDUMBparms(int pid, int tid, int p1, int p2);
	//--
	EXPORT void saveCoreLoggerParms(int pid_, int tid_, int readFrom, bool saveToDB, bool saveToFile, bool saveMSEFlag, bool saveRunFlag, bool saveInternalsFlag, bool saveImageFlag);
	EXPORT void loadCoreLoggerParms(int pid_, int tid_, int* readFrom, bool* saveToDB, bool* saveToFile, bool* saveMSEFlag, bool* saveRunFlag, bool* saveInternalsFlag, bool* saveImageFlag);
	//--
	EXPORT void loadCoreNNparms(int pid, int tid, char** levelRatioS_, char** levelActivationS_, bool* useContext_, bool* useBias_, int* maxEpochs_, float* targetMSE_, int* netSaveFrequency_, bool* stopOnDivergence_, int* BPalgo_, float* learningRate_, float* learningMomentum_);
	EXPORT void loadCoreGAparms(int pid, int tid, int p1, int p2);
	EXPORT void loadCoreSOMparms(int pid, int tid, int p1, int p2);
	EXPORT void loadCoreSVMparms(int pid, int tid, int p1, int p2);
	EXPORT void loadCoreDUMBparms(int pid, int tid, int p1, int p2);
	//--
	EXPORT void saveCoreNNInternalsSCGD(int pid_, int tid_, int iterationsCnt_, numtype* delta_, numtype* mu_, numtype* alpha_, numtype* beta_, numtype* lambda_, numtype* lambdau_, numtype* Gtse_old_, numtype* Gtse_new_, numtype* comp_, numtype* pnorm_, numtype* rnorm_, numtype* dwnorm_);
	//--
	EXPORT void loadDBConnInfo(int pid_, int tid_, char** oDBusername, char** oDBpassword, char** oDBconnstring);
	EXPORT void saveDBConnInfo(int pid_, int tid_, char* oDBusername, char* oDBpassword, char* oDBconnstring);
	//--
	EXPORT void saveTradeInfo(int MT4clientPid, int MT4sessionId, int MT4accountId, int MT4enginePid, int iPositionTicket, char* iPositionOpenTime, char* iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iLastForecastO, double iLastForecastH, double iLastForecastL, double iLastForecastC, double iLastForecastV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult, int iTPhit, int iSLhit);
	//--
	EXPORT void saveXMLconfig(int simulationId_, int pid_, int tid_, int fileId_, sCfg* cfg_);
};