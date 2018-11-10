#include "sClientLogger.h"

sClientLogger::sClientLogger(sCfgObjParmsDef) : sLogger(sCfgObjParmsVal) {
	logsCnt=1;

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &saveToDB, "saveToDB");
	safecall(cfgKey, getParm, &saveToFile, "saveToFile");
	safecall(cfgKey, getParm, &saveClientInfoFlag, "saveClient");

	//-- 2. do stuff and spawn sub-Keys

	//-- spawn destination OraData
	safecall(cfgKey, getParm, &saveToDB, "saveToDB");
	if (saveToDB) safespawn(oradb, newsname("Persistor_OraData"), defaultdbg, cfg, "DestOraData");
	//-- spawn destination FileData
	safecall(cfgKey, getParm, &saveToFile, "saveToFile");
	if (saveToFile) {
		//filedb= new sFileData(this, newsname("Persistor_FileData"), defaultdbg, FILE_MODE_WRITE, true);
		safespawn(filedb, newsname("Persistor_FileData"), defaultdbg, FILE_MODE_WRITE, true);
	}
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sClientLogger::~sClientLogger(){}

void sClientLogger::saveClientInfo(int pid, const char* clientName, double startTime, double elapsedSecs, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun) {
	if (saveToDB) {
		safecall(oradb, saveClientInfo, pid, clientName, startTime, elapsedSecs, simulLen, simulStart, doTrain, doTrainRun, doTestRun);
	}
	if (saveToFile) {
		safecall(filedb, saveClientInfo, pid, clientName, startTime, elapsedSecs, simulLen, simulStart, doTrain, doTrainRun, doTestRun);
	}
}
