#include "sLogger.h"

sLogger::sLogger(sObjParmsDef) : sCfgObj(sObjParmsVal, nullptr, nullptr) {}
sLogger::sLogger(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &saveToDB, "saveToDB");
	safecall(cfgKey, getParm, &saveToFile, "saveToFile");
	
	//-- 2. do stuff and spawn sub-Keys
	if (saveToDB) safespawn(oradb, newsname("Logger_OraData"), defaultdbg, cfg, "OraData");
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sLogger::~sLogger() {}

void sLogger::open() {
	if (saveToDB) safecall(oradb, open);
	if (saveToFile) safecall(filedb, open, FILE_MODE_WRITE);
}

void sLogger::saveClientInfo(int pid, int simulationId, const char* clientName, double startTime, double elapsedSecs, char* simulStartTrain, char* simulStartInfer, char* simulStartValid, bool doTrain, bool doTrainRun, bool doTestRun) {
	if (saveToDB) safecall(oradb, saveClientInfo, pid, simulationId, clientName, startTime, elapsedSecs, simulStartTrain, simulStartInfer, simulStartValid, doTrain, doTrainRun, doTestRun);
	if (saveToFile) safecall(filedb, saveClientInfo, pid, simulationId, clientName, startTime, elapsedSecs, simulStartTrain, simulStartInfer, simulStartValid, doTrain, doTrainRun, doTestRun);
}
void sLogger::loadEngineInfo(int pid, int* engineType, int* coresCnt, int* coreId, int* coreType, int* parentCoresCnt, int** parentCore, int** parentConnType) {
	if (saveToDB) safecall(oradb, loadEngineInfo, pid, engineType, coresCnt, coreId, coreType, parentCoresCnt, parentCore, parentConnType);
//	if (saveToFile) safecall(filedb, loadEngineInfo, pid, engineType, coresCnt, coreId, coreType, parentCoresCnt, parentCore, parentConnType);
}