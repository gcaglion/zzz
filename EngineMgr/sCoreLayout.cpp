#include "sCoreLayout.h"

sCore::sCore(sCfgObjParmsDef, int id_, sDataShape* dataShape_) : sCfgObj(sCfgObjParmsVal) {
	id=id_; baseDataShape=dataShape_;

	//-- 0. mallocs
	parentId=(int*)malloc(CORE_MAX_PARENTS*sizeof(int));
	parentConnType=(int*)malloc(CORE_MAX_PARENTS*sizeof(int));
	parentDesc=(char**)malloc(CORE_MAX_PARENTS*sizeof(char*)); for (int p=0; p<CORE_MAX_PARENTS; p++) parentDesc[p]=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &type, "Type");
	safecall(cfgKey, getParm, &parentDesc, "Parents", false, &parentsCnt);
	safecall(cfgKey, getParm, &parentConnType, "ParentsConnType", false, &parentsCnt);
	//-- 2. do stuff : extract parentId from each parentDesc
	for (int p=0; p<parentsCnt; p++) parentId[p]=atoi(right(parentDesc[p], (int)strlen(parentDesc[p])-4));	
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;	


}
sCore::~sCore() {
	free(parentId);
	free(parentConnType);
	for (int p=0; p<CORE_MAX_PARENTS; p++) free(parentDesc[p]); 
	free(parentDesc);
}

//void sCore::init(int coreId_, sDataShape* dataShape_, void* coreParms_) {}