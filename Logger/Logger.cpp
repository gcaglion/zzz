#include "Logger.h"

sLogger::sLogger(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	saveNothingFlag=false; saveClientFlag=true; saveMSEFlag=true; saveRunFlag=true; saveInternalsFlag=false; saveImageFlag=false;
	ffname_malloc();
	//-- 1. get Parameters


	safecall(cfgKey, getParm, &saveNothingFlag, "SaveNothing", true);
	if(!saveNothingFlag) {
		safecall(cfgKey, getParm, &saveClientFlag, "SaveClient", true);
		safecall(cfgKey, getParm, &saveMSEFlag, "SaveMSE", true);
		safecall(cfgKey, getParm, &saveRunFlag, "SaveRun", true);
		safecall(cfgKey, getParm, &saveInternalsFlag, "SaveInternals", true);
		safecall(cfgKey, getParm, &saveImageFlag, "SaveImage", true);
		//-- get destination DB
		if (!safespawn(true, db, newsname("PersistorDB"), defaultdbg, cfg, "DestDB", false)) db=nullptr;
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

void sLogger::saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV) {
}
void sLogger::saveRun(int pid, int tid, int setid, int npid, int ntid, int runCnt, int featuresCnt, int* feature, numtype* prediction, numtype* actual) {}
void sLogger::saveW(int pid, int tid, int epoch, int Wcnt, numtype* W) {}
void sLogger::loadW(int pid, int tid, int epoch, int Wcnt, numtype* W) {}
void sLogger::saveClient(int pid, char* clientName, DWORD startTime, DWORD duration, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun) {}
void sLogger::commit() {}
