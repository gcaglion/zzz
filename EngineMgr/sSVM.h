#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sSVMparms.h"

struct sSVM : sCore {
	EXPORT sSVM(sObjParmsDef, int inputCnt_, int outputCnt_, sCoreLogger* persistor_, sSVMparms* SVMparms_);
	EXPORT sSVM(sCfgObjParmsDef, int inputCnt_, int outputCnt_, sSVMparms* SVMparms_);
	EXPORT ~sSVM();

	//-- local implementations of sCore virtual methods
	void setLayout();
	void mallocLayout();
	void train();
	void infer();
	void saveImage(int pid, int tid, int epoch);
	void loadImage(int pid, int tid, int epoch);
};