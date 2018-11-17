#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sSOMparms.h"

struct sSOM : sCore {
	EXPORT sSOM(sCfgObjParmsDef, sCoreLayout* layout_, sSOMparms* SOMparms_);
	EXPORT ~sSOM();

	//-- local implementations of sCore virtual methods
	void setLayout(int batchSamplesCnt_);
	void mallocLayout();
	void train(sCoreProcArgs* trainArgs);
	void singleInfer(int sampleLen_, int sampleFeaturesCnt_, int batchSamplesCnt_, numtype* singleSampleBF, numtype* singleTargetBF, numtype** singlePredictionBF);
	void saveImage(int pid, int tid, int epoch);
	void loadImage(int pid, int tid, int epoch);
};