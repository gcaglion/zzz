#include "sFileData.h"

sFileData::sFileData(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

//--------- TO FIX !! --------------
	/*
	//-- 1. get Parameters
	safecall(cfgKey, getParm, &filesCnt, "FilesCount");
	for(int f=0)
	safecall(cfgKey, getParm, &fileFullName[0], "Client", true);
	safecall(cfgKey, getParm, &fileFullName[1], "MSE", true);
	safecall(cfgKey, getParm, &fileFullName[2], "Run", true);
	safecall(cfgKey, getParm, &fileFullName[3], "Internals", true);
	safecall(cfgKey, getParm, &fileFullName[4], "Image", true);
	//-- 2. do stuff and spawn sub-Keys
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
*/
	cfg->currentKey=bkpKey;

}
sFileData::~sFileData() {}

void sFileData::getStartDates(sFileData* dateSource_, char* startDate_, int datesCnt_, char** oDate_) {}

void sFileData::ffname_malloc() {
	fileFullName=(char**)malloc(5*sizeof(char*));
	for (int f=0; f<5; f++) fileFullName[f]=(char*)malloc(MAX_PATH);
}
