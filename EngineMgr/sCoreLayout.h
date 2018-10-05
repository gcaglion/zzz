#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/DataShape.h"
#include "Core_enums.h"

#define CORE_MAX_PARENTS	32

struct sCore : sCfgObj {
	int id;
	int type;
	int layer=-1;
	int parentsCnt;
	int* parentId;
	int* parentConnType;

	sDataShape* baseDataShape;

	EXPORT sCore(sCfgObjParmsDef, int id_, sDataShape* dataShape_);
	EXPORT ~sCore();
	virtual void init(int coreId_, sDataShape* dataShape_, void* coreParms_);

private:
	char** parentDesc;

};
