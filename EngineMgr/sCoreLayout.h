#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"

#define CORE_MAX_PARENTS 32

struct sCoreLayout : sCfgObj {
	int type;	//-- core type
	int layer=-1;
	int parentsCnt;
	int* parentId;
	int* parentConnType;
	char** parentDesc;
	//-- data shape
	int inputCnt;
	int outputCnt;


	EXPORT sCoreLayout(sCfgObjParmsDef, int inputCnt_, int outputCnt_);
	EXPORT ~sCoreLayout();

};