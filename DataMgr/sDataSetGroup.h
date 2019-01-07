#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "sDataSet.h"

struct sDataSetGroup : sCfgObj {
	int dataSetsCnt;
	sDataSet** ds;
	int inputCnt;
	int outputCnt;

	EXPORT sDataSetGroup(sObjParmsDef, int dataSetsCnt_, sDataSet** ds_);
	EXPORT sDataSetGroup(sCfgObjParmsDef, int extraSteps=0);
	EXPORT ~sDataSetGroup();

	void sDataSetGroup_pre();
	void sDataSetGroup_post();
};