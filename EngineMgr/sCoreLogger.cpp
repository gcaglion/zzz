#include "sCoreLogger.h"

sCoreLogger::sCoreLogger(sObjParmsDef, int readFrom_, bool saveToDB_, bool saveToFile_, bool saveMSEFlag_, bool saveRunFlag_, bool saveInternalsFlag_, bool saveImageFlag_) : sLogger(sObjParmsVal, readFrom_, saveToDB_, saveToFile_) {
	saveToDB=saveToDB_; saveToFile=saveToFile_;
	saveMSEFlag=saveMSEFlag_;
	saveRunFlag=saveRunFlag_;
	saveInternalsFlag=saveInternalsFlag_;
	saveImageFlag=saveImageFlag_;
}
sCoreLogger::sCoreLogger(sCfgObjParmsDef) : sLogger(sCfgObjParmsVal) {
	logsCnt=4; mallocs();
	oradb=nullptr;  filedb=nullptr;
	//-- 1. get Parameters
	safecall(cfgKey, getParm, &saveMSEFlag, "SaveMSE");
	safecall(cfgKey, getParm, &saveRunFlag, "SaveRun");
	safecall(cfgKey, getParm, &saveInternalsFlag, "SaveInternals");
	safecall(cfgKey, getParm, &saveImageFlag, "SaveImage");

	safecall(cfgKey, getParm, &saveToDB, "saveToDB");
	//-- spawn destination OraData
	if (saveToDB) safespawn(oradb, newsname("Persistor_OraData"), defaultdbg, cfg, "Persistor/OraData");

	safecall(cfgKey, getParm, &saveToFile, "saveToFile");
	//-- spawn destination FileData
	if (saveToFile) {
		safecall(cfgKey, getParm, &ffn[0], "DestFileData/MSE");
		safecall(cfgKey, getParm, &ffn[1], "DestFileData/Run");
		safecall(cfgKey, getParm, &ffn[2], "DestFileData/Internals");
		safecall(cfgKey, getParm, &ffn[3], "DestFileData/Image");

		safespawn(filedb, newsname("Persistor_FileData"), defaultdbg, FILE_MODE_WRITE, true);
	}

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;


}
sCoreLogger::sCoreLogger(sObjParmsDef, sLogger* persistor_, int pid_, int tid_) : sLogger(sObjParmsVal, 0, false, false) {

	//-- set persistor parameters
	safecall(persistor_, loadCoreLoggerParms, pid_, tid_, &readFrom, &saveToDB, &saveToFile, &saveMSEFlag, &saveRunFlag, &saveInternalsFlag, &saveImageFlag);
	//-- spawn oradb
	if (saveToDB) {
		char DBusername[DBUSERNAME_MAXLEN]; char* pDBusername=&DBusername[0];
		char DBpassword[DBPASSWORD_MAXLEN]; char* pDBpassword=&DBpassword[0];
		char DBconnstring[DBCONNSTRING_MAXLEN]; char* pDBconnstring=&DBconnstring[0];
		safecall(persistor_, loadDBConnInfo, pid_, tid_, &pDBusername, &pDBpassword, &pDBconnstring);

	}
}
sCoreLogger::~sCoreLogger(){
	for (int f=0; f<logsCnt; f++) free(ffn[f]);
	free(ffn);
}
void sCoreLogger::mallocs() {
	ffn=(char**)malloc(4*sizeof(char*));
	for (int f=0; f<logsCnt; f++) ffn[f]=(char*)malloc(MAX_PATH);
}

void sCoreLogger::save(sLogger* persistor_, int pid_, int tid_){
	safecall(persistor_, saveCoreLoggerParms, pid_, tid_, readFrom, saveToDB, saveToFile, saveMSEFlag, saveRunFlag, saveInternalsFlag, saveImageFlag);
}
