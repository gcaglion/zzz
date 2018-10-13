#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/DataShape.h"
#include "../DataMgr/DataSet.h"
#include "Core_enums.h"

#define CORE_MAX_PARENTS	32

struct sCore : sCfgObj {
	int type;
	int layer=-1;
	int parentsCnt;
	int* parentId;
	int* parentConnType;

	sDataShape* baseDataShape;

	sDataSet* inputDS;
	sDataSet* outputDS;

	EXPORT sCore(sCfgObjParmsDef, sDataShape* dataShape_);
	EXPORT ~sCore();

	void loadInput(sDataSet* inputDS_);

private:
	char** parentDesc;

};
