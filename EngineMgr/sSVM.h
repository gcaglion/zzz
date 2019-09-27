#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sSVMparms.h"

struct sSVM : sCore {
	EXPORT sSVM(sObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_, sSVMparms* SVMparms_);
	EXPORT sSVM(sCfgObjParmsDef, sCoreLayout* layout_, sSVMparms* SVMparms_);
	EXPORT ~sSVM();

	//-- local implementations of sCore virtual methods
	void setLayout(int batchSamplesCnt_);
	void mallocLayout();
	void train(sCoreProcArgs* trainArgs);
	void infer(sCoreProcArgs* inferArgs);
	void saveImage(int pid, int tid, int epoch);
	void loadImage(int pid, int tid, int epoch);
};