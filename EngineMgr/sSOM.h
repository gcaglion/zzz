#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sSOMparms.h"

struct sSOM : sCore {

	EXPORT sSOM(sObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sCoreLogger* persistor_, sSOMparms* SOMparms_);
	EXPORT sSOM(sCfgObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sSOMparms* SOMparms_);
	EXPORT ~sSOM();

	//-- local implementations of sCore virtual methods
	void setLayout(int batchSamplesCnt_);
	void mallocLayout();
	void train(sCoreProcArgs* trainArgs);
	void infer(sCoreProcArgs* inferArgs);
	void saveImage(int pid, int tid, int epoch);
	void loadImage(int pid, int tid, int epoch);
};