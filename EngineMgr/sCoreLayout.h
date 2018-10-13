#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"

#define CORE_MAX_PARENTS 32

struct sCoreLayout : sCfgObj {
	int layer=-1;
	int parentsCnt;
	int* parentId;
	int* parentConnType;
	char** parentDesc;

	EXPORT sCoreLayout(sCfgObjParmsDef);
	EXPORT ~sCoreLayout();

};