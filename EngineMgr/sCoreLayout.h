#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../Logger/sLogger.h"

#define CORE_MAX_PARENTS 32

struct sCoreLayout : sCfgObj {
	int type;	//-- core type
	int tid;	//-- thread id. 
	int layer=-1;
	int parentsCnt;
	int* parentId;
	int* parentConnType;
	char** parentDesc;
	//-- data shape
	int inputCnt;
	int outputCnt;


	EXPORT sCoreLayout(sObjParmsDef, int inputCnt_, int outputCnt_, int type_, int parentsCnt_, int* parentId_, int* parentConnType_, int tid_=0);
	EXPORT sCoreLayout(sCfgObjParmsDef, int inputCnt_, int outputCnt_, int tid_=0);
	EXPORT sCoreLayout(sObjParmsDef, sLogger* persistor_, int pid_, int coreId_);
	EXPORT ~sCoreLayout();

	EXPORT void save(sLogger* persistor_, int pid_, int tid_);
};