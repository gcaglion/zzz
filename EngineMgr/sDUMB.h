#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sDUMBparms.h"

struct sDUMB : sCore {

	float fixedTRSerror;

	EXPORT sDUMB(sObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_, sDUMBparms* DUMBparms_);
	EXPORT sDUMB(sCfgObjParmsDef, sCoreLayout* layout_, sDUMBparms* DUMBparms_);
	EXPORT ~sDUMB();

	//-- local implementations of sCore virtual methods
	void setLayout();
	void mallocLayout();
	void train();
	void infer();
	void saveImage(int pid, int tid, int epoch);
	void loadImage(int pid, int tid, int epoch);
};