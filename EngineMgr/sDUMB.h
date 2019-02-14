#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sDUMBparms.h"

struct sDUMB : sCore {

	float fixedTRSerror;

	EXPORT sDUMB(sObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sCoreLogger* persistor_, sDUMBparms* DUMBparms_);
	EXPORT sDUMB(sCfgObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sDUMBparms* DUMBparms_);
	EXPORT ~sDUMB();

	//-- local implementations of sCore virtual methods
	void setLayout(int batchSamplesCnt_);
	void mallocLayout();
	void train(sCoreProcArgs* trainArgs);
	void infer(sCoreProcArgs* inferArgs);
	void saveImage(int pid, int tid, int epoch);
	void loadImage(int pid, int tid, int epoch);
};