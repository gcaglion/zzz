#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sGAparms.h"

struct sGA : sCore {

	EXPORT sGA(sObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sCoreLogger* persistor_, sGAparms* GAparms_);
	EXPORT sGA(sCfgObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sGAparms* GAparms_);
	EXPORT ~sGA();

	//-- local implementations of sCore virtual methods
	void setLayout(int batchSamplesCnt_);
	void mallocLayout();
	void train(sCoreProcArgs* trainArgs);
	void infer(sCoreProcArgs* inferArgs);
	void saveImage(int pid, int tid, int epoch);
	void loadImage(int pid, int tid, int epoch);
};