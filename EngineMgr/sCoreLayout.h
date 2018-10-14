#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/DataShape.h"

#define CORE_MAX_PARENTS 32

struct sCoreLayout : sCfgObj {
	int type;	//-- core type
	int layer=-1;
	int parentsCnt;
	int* parentId;
	int* parentConnType;
	char** parentDesc;
	sDataShape* shape;


	EXPORT sCoreLayout(sCfgObjParmsDef, sDataShape* shape_);
	EXPORT ~sCoreLayout();

};