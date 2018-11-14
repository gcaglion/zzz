#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../FileData/sFileData.h" 
#include "../OraData/sOraData.h"

struct sLogger : sCfgObj {

	//-- How many logs
	int logsCnt;

	//-- Where
	bool saveToDB;
	bool saveToFile;
	sOraData* oradb;
	sFileData* filedb;

	EXPORT sLogger(sObjParmsDef, bool saveToDB_, bool saveToFile_);	//-- this is called by one of sCoreLogger constructors
	EXPORT sLogger(sObjParmsDef, sOraData* oradb_);
	EXPORT sLogger(sObjParmsDef, sFileData* filedb_);
	EXPORT sLogger(sObjParmsDef, sOraData* oradb_, sFileData* filedb_);
	EXPORT sLogger(sCfgObjParmsDef);
	EXPORT ~sLogger();

	EXPORT virtual void open();
	EXPORT virtual void commit() {}

	//-- not sure it should be here, this is temporary
	EXPORT void saveClientInfo(int pid, int simulationId, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doTrainRun, bool doTestRun);
	EXPORT void loadEngineInfo(int pid, int* engineType, int* coresCnt, int* coreId, int* coreType, int* parentCoresCnt, int** parentCore, int** parentConnType);

	//--
	EXPORT void saveCoreNNparms(int pid, int tid, char* levelRatioS_, char* levelActivationS_, bool useContext_, bool useBias_, int maxEpochs_, numtype targetMSE_, int netSaveFrequency_, bool stopOnDivergence_, int BPalgo_, float learningRate_, float learningMomentum_);
	EXPORT void saveCoreGAparms(int pid, int tid, int p1, int p2);
	EXPORT void saveCoreSOMparms(int pid, int tid, int p1, int p2);
	EXPORT void saveCoreSVMparms(int pid, int tid, int p1, int p2);
	EXPORT void saveCoreDUMBparms(int pid, int tid, int p1, int p2);
	//--
	EXPORT void loadCoreNNparms(int pid, int tid, char** levelRatioS_, char** levelActivationS_, bool* useContext_, bool* useBias_, int* maxEpochs_, numtype* targetMSE_, int* netSaveFrequency_, bool* stopOnDivergence_, int* BPalgo_, float* learningRate_, float* learningMomentum_);

};