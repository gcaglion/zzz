#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sGAparms.h"

struct sGA : sCore {

	EXPORT sGA(sObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_, sGAparms* GAparms_);
	EXPORT sGA(sCfgObjParmsDef, sCoreLayout* layout_, sGAparms* GAparms_);
	EXPORT ~sGA();

	//-- local implementations of sCore virtual methods
	void setLayout();
	void mallocLayout();
	void train();
	void infer();
	void saveImage(int pid, int tid, int epoch);
	void loadImage(int pid, int tid, int epoch);
};