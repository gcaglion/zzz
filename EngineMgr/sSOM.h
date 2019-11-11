#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sSOMparms.h"

struct sSOM : sCore {

	EXPORT sSOM(sObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_, sSOMparms* SOMparms_);
	EXPORT sSOM(sCfgObjParmsDef, sCoreLayout* layout_, sSOMparms* SOMparms_);
	EXPORT ~sSOM();

	//-- local implementations of sCore virtual methods
	void setLayout();
	void mallocLayout();
	void train();
	void infer();
	void saveImage(int pid, int tid, int epoch);
	void loadImage(int pid, int tid, int epoch);
};