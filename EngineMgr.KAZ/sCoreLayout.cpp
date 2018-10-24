#include "sCoreLayout.h"

sCoreLayout::sCoreLayout(sCfgObjParmsDef, sDataShape* shape_) : sCfgObj(sCfgObjParmsVal) {
	shape=shape_;
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
sCoreLayout::~sCoreLayout() {
	free(parentId);
	free(parentConnType);
	for (int p=0; p<CORE_MAX_PARENTS; p++) free(parentDesc[p]);
	free(parentDesc);
}