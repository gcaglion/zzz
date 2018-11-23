#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sDUMBparms.h"

struct sDUMB : sCore {

	float fixedTRSerror;

	EXPORT sDUMB(sCfgObjParmsDef, sCoreLayout* layout_, sDUMBparms* DUMBparms_);
	EXPORT ~sDUMB();

	//-- local implementations of sCore virtual methods
	void setLayout(int batchSamplesCnt_);
	void mallocLayout();
	void train(sCoreProcArgs* trainArgs);
	void infer(sCoreProcArgs* inferArgs);
	void singleInfer(int sampleLen_, int sampleFeaturesCnt_, int batchSamplesCnt_, numtype* singleSampleBF, numtype* singleTargetBF, numtype** singlePredictionBF);
	void saveImage(int pid, int tid, int epoch);
	void loadImage(int pid, int tid, int epoch);
};