#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sSVMparms.h"

struct sSVM : sCore {
	EXPORT sSVM(sCfgObjParmsDef, sCoreLayout* layout_, sSVMparms* SVMparms_);
	EXPORT ~sSVM();

	//-- local implementations of sCore virtual methods
	void setLayout(int batchSamplesCnt_);
	void mallocLayout();
	void train(sCoreProcArgs* trainArgs);
	void singleInfer(numtype* singleSampleSBF, numtype* singleTargetSBF, numtype** singlePredictionSBF);
};