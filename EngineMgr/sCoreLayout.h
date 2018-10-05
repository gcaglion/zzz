#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/DataShape.h"

#define CORE_MAX_PARENTS	32

struct sCoreLayout : sCfgObj {
	int id;
	int type;
	int layer=-1;
	int parentsCnt;
	int* parentId;
	int* parentConnType;

	sDataShape* dataShape;

	EXPORT sCoreLayout(sCfgObjParmsDef, int id_, sDataShape* dataShape_);
	EXPORT ~sCoreLayout();

private:
	char** parentDesc;

};
