#include "sCoreLogger.h"

sCoreLogger::sCoreLogger(sObjParmsDef, bool saveToDB_, bool saveToFile_, bool saveMSEFlag_, bool saveRunFlag_, bool saveInternalsFlag_, bool saveImageFlag_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	saveMSEFlag=saveMSEFlag_;
	saveRunFlag=saveRunFlag_;
	saveInternalsFlag=saveInternalsFlag_;
	saveImageFlag=saveImageFlag_;
}
sCoreLogger::sCoreLogger(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	saveMSEFlag=true; saveRunFlag=true; saveInternalsFlag=false; saveImageFlag=false;
	//-- 1. get Parameters
	safecall(cfgKey, getParm, &saveToDB, "saveToDB", true);
	safecall(cfgKey, getParm, &saveToFile, "saveToFile", true);
	safecall(cfgKey, getParm, &saveMSEFlag, "SaveMSE", true);
	safecall(cfgKey, getParm, &saveRunFlag, "SaveRun", true);
	safecall(cfgKey, getParm, &saveInternalsFlag, "SaveInternals", true);
	safecall(cfgKey, getParm, &saveImageFlag, "SaveImage", true);
	//-- spawn destination OraData
	if (saveToDB) safespawn(db, newsname("Persistor_OraData"), defaultdbg, cfg, "DestOraData", false);
	//-- spawn destination FileData
	if (saveToFile) safespawn(db, newsname("Persistor_FileData"), defaultdbg, cfg, "DestFileData", false); 
	/*{
	}*/

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;


}
sCoreLogger::~sCoreLogger(){
}

void sCoreLogger::saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV){}