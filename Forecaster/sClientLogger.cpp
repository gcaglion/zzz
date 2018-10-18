#include "sClientLogger.h"

sClientLogger::sClientLogger(sCfgObjParmsDef) : sLogger(sCfgObjParmsVal) {
	logsCnt=1;

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &saveToDB, "saveToDB");
	safecall(cfgKey, getParm, &saveToFile, "saveToFile");
	safecall(cfgKey, getParm, &saveClientInfoFlag, "saveClient");

	//-- 2. do stuff and spawn sub-Keys

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sClientLogger::~sClientLogger(){}