#include "sCoreLayout.h"

sCoreLayout::sCoreLayout(sObjParmsDef, int inputCnt_, int outputCnt_, int type_, int parentsCnt_, int* parentId_, int* parentConnType_, int tid_) : sCfgObj(sObjParmsVal, nullptr, "") {
	parentsCnt=parentsCnt_; inputCnt=inputCnt_; outputCnt=outputCnt_;
	tid=(tid_==0) ? GetCurrentThreadId() : tid_;
	//-- 0. mallocs
	parentId=(int*)malloc(CORE_MAX_PARENTS*sizeof(int));
	parentConnType=(int*)malloc(CORE_MAX_PARENTS*sizeof(int));
	parentDesc=(char**)malloc(CORE_MAX_PARENTS*sizeof(char*)); for (int p=0; p<CORE_MAX_PARENTS; p++) parentDesc[p]=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);
	//-- 1. get Parameters
	type=type_;
	for (int p=0; p<parentsCnt; p++) {
		parentId[p]=parentId_[p];
		parentConnType[p]=parentConnType_[p];
	}
}
sCoreLayout::sCoreLayout(sCfgObjParmsDef, int inputCnt_, int outputCnt_, int tid_) : sCfgObj(sCfgObjParmsVal) {
	inputCnt=inputCnt_; outputCnt=outputCnt_;
	tid=(tid_==0) ? GetCurrentThreadId() : tid_;
	//-- 0. mallocs
	parentId=(int*)malloc(CORE_MAX_PARENTS*sizeof(int));
	parentConnType=(int*)malloc(CORE_MAX_PARENTS*sizeof(int));
	parentDesc=(char**)malloc(CORE_MAX_PARENTS*sizeof(char*)); for (int p=0; p<CORE_MAX_PARENTS; p++) parentDesc[p]=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);
	//-- 1. get Parameters
	safecall(cfgKey, getParm, &type, "Type", false);
	safecall(cfgKey, getParm, &parentDesc, "Parents", false, &parentsCnt);
	safecall(cfgKey, getParm, &parentConnType, "ParentsConnType", false, &parentsCnt);
	//-- 2. do stuff : extract parentId from each parentDesc
	for (int p=0; p<parentsCnt; p++) parentId[p]=atoi(right(parentDesc[p], (int)strlen(parentDesc[p])-4));
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;
}
sCoreLayout::sCoreLayout(sObjParmsDef, sLogger* persistor_, int pid_, int coreId_) : sCfgObj(sObjParmsVal, nullptr, "") {
	//-- 0. mallocs
	parentId=(int*)malloc(CORE_MAX_PARENTS*sizeof(int));
	parentConnType=(int*)malloc(CORE_MAX_PARENTS*sizeof(int));
	parentDesc=(char**)malloc(CORE_MAX_PARENTS*sizeof(char*)); for (int p=0; p<CORE_MAX_PARENTS; p++) parentDesc[p]=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);
	//-- 1. load from db
	//persistor_->loadCoreLayout(pid_, coreId_)
}
sCoreLayout::~sCoreLayout() {
	free(parentId);
	free(parentConnType);
	for (int p=0; p<CORE_MAX_PARENTS; p++) free(parentDesc[p]);
	free(parentDesc);
}

void sCoreLayout::save(sLogger* persistor_, int pid_, int tid_) {

}