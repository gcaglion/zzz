#include "Logger.h"

/*sLogger::sLogger(sObjParmsDef, sOraConnection* db_, bool saveNothing_, bool saveClient_, bool saveMSE_, bool saveRun_, bool saveInternals_, bool saveImage_) : sCfgObj(sCfgObjParmsVal) {
	dest=LOG_TO_ORCL; db=db_;  saveNothing=saveNothing_; saveClient=saveClient_; saveMSE=saveMSE_; saveRun=saveRun_; saveInternals=saveInternals_; saveImage=saveImage_;
}
sLogger::sLogger(sObjParmsDef, FILE* fileH_, bool saveNothing_, bool saveClient_, bool saveMSE_, bool saveRun_, bool saveInternals_, bool saveImage_) : sObj(sObjParmsVal) {
	dest=LOG_TO_TEXT; fileH=fileH_;  saveNothing=saveNothing_; saveClient=saveClient_; saveMSE=saveMSE_; saveRun=saveRun_; saveInternals=saveInternals_; saveImage=saveImage_;
}*/
sLogger::sLogger(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	saveNothing=false; saveClient=true; saveMSE=true; saveRun=true; saveInternals=false; saveImage=false;
	ffname_malloc();
	//-- 1. get Parameters


	safecall(cfgKey, getParm, &saveNothing, "SaveNothing", true);
	if(!saveNothing) {
		safecall(cfgKey, getParm, &saveClient, "SaveClient", true);
		safecall(cfgKey, getParm, &saveMSE, "SaveMSE", true);
		safecall(cfgKey, getParm, &saveRun, "SaveRun", true);
		safecall(cfgKey, getParm, &saveInternals, "SaveInternals", true);
		safecall(cfgKey, getParm, &saveImage, "SaveImage", true);
		//-- get destination DB
		if (!safespawn(true, db, newsname("PersistorDB"), defaultdbg, cfg, "DestDB")) db=nullptr;
		//-- get destination files
		safecall(cfgKey, getParm, &fileFullName[0], "Client", true);
		safecall(cfgKey, getParm, &fileFullName[1], "MSE", true);
		safecall(cfgKey, getParm, &fileFullName[2], "Run", true);
		safecall(cfgKey, getParm, &fileFullName[3], "Internals", true);
		safecall(cfgKey, getParm, &fileFullName[4], "Image", true);
	}
	//-- 2. do stuff and spawn sub-Keys
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sLogger::~sLogger() {
	for (int f=0; f<5; f++) free(fileFullName[f]);
	free(fileFullName);
}

void sLogger::ffname_malloc() {
	fileFullName=(char**)malloc(5*sizeof(char*));
	for (int f=0; f<5; f++) fileFullName[f]=(char*)malloc(MAX_PATH);
}

void sLogger::SaveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV) {
}
void sLogger::SaveRun(int pid, int tid, int setid, int npid, int ntid, int runCnt, int featuresCnt, int* feature, numtype* prediction, numtype* actual) {}
void sLogger::SaveW(int pid, int tid, int epoch, int Wcnt, numtype* W) {}
void sLogger::LoadW(int pid, int tid, int epoch, int Wcnt, numtype* W) {}
void sLogger::SaveClient(int pid, char* clientName, DWORD startTime, DWORD duration, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun) {}
void sLogger::Commit() {}
