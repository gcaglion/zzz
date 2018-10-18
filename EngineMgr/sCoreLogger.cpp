#include "sCoreLogger.h"

sCoreLogger::sCoreLogger(sObjParmsDef, bool saveToDB_, bool saveToFile_, bool saveMSEFlag_, bool saveRunFlag_, bool saveInternalsFlag_, bool saveImageFlag_) : sLogger(sObjParmsVal, nullptr, nullptr) {
	saveToDB=saveToDB_; saveToFile=saveToFile_;
	saveMSEFlag=saveMSEFlag_;
	saveRunFlag=saveRunFlag_;
	saveInternalsFlag=saveInternalsFlag_;
	saveImageFlag=saveImageFlag_;
}
sCoreLogger::sCoreLogger(sCfgObjParmsDef) : sLogger(sCfgObjParmsVal) {
	logsCnt=4; mallocs();
	db=nullptr;  file=nullptr;
	//-- 1. get Parameters
	safecall(cfgKey, getParm, &saveMSEFlag, "SaveMSE");
	safecall(cfgKey, getParm, &saveRunFlag, "SaveRun");
	safecall(cfgKey, getParm, &saveInternalsFlag, "SaveInternals");
	safecall(cfgKey, getParm, &saveImageFlag, "SaveImage");

	safecall(cfgKey, getParm, &saveToDB, "saveToDB");
	//-- spawn destination OraData
	if (saveToDB) safespawn(db, newsname("Persistor_OraData"), defaultdbg, cfg, "DestOraData", true);

	safecall(cfgKey, getParm, &saveToFile, "saveToFile");
	//-- spawn destination FileData
	if (saveToFile) {
		safecall(cfgKey, getParm, &ffn[0], "DestFileData/MSE");
		safecall(cfgKey, getParm, &ffn[1], "DestFileData/Run");
		safecall(cfgKey, getParm, &ffn[2], "DestFileData/Internals");
		safecall(cfgKey, getParm, &ffn[3], "DestFileData/Image");
		safespawn(file, newsname("Persistor_FileData"), defaultdbg, cfg, "DestFileData", FILE_MODE_WRITE, true, logsCnt, ffn);
	}

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;


}
sCoreLogger::~sCoreLogger(){
	for (int f=0; f<logsCnt; f++) free(ffn[f]);
	free(ffn);
}
void sCoreLogger::mallocs() {
	ffn=(char**)malloc(4*sizeof(char*));
	for (int f=0; f<logsCnt; f++) ffn[f]=(char*)malloc(MAX_PATH);
}
void sCoreLogger::loadW(int pid, int tid, int epoch, int Wcnt, numtype* W) {}

void sCoreLogger::saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV) {
	if (saveToDB) {
		safecall(db, saveMSE, pid, tid, mseCnt, mseT, mseV);
	}
	if (saveToFile) {
		safecall(file, saveMSE, pid, tid, mseCnt, mseT, mseV);
	}
}