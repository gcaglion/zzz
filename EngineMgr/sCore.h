#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/sDataSet.h"
#include "sCoreLayout.h"
#include "sCoreParms.h"
#include "sCoreLogger.h"
#include "Core_enums.h"

//-- properties in sCoreProcArgs are common across core types
struct sCoreProcArgs {
	//-- common across train and infer
	sDataSet* ds;
	int pid;
	int tid;
	int testid;
	int screenLine;
	//-- train-specific
	int mseCnt;
	numtype* mseT;
	numtype* mseV;
	//-- infer-specific
	int npid;
	int ntid;
	int runCnt;
	int featuresCnt;
	int* feature;
	numtype* actualTRS;
	numtype* predictedTRS;
	numtype* actual;
	numtype* predicted;
	numtype* scaleM;
	numtype* scaleP;
};

struct sCore : sCfgObj {

	//int pid, tid;

	sCoreParms* parms;
	sCoreLayout* layout;
	sCoreLogger* persistor;
	sCoreProcArgs* procArgs;

	EXPORT sCore(sCfgObjParmsDef, sCoreLayout* layout_);
	EXPORT ~sCore();

	//-- methods to be implemented indipendently by each subclass (sNN, sGA, ...)
	virtual void train(sCoreProcArgs* procArgs_){}
	virtual void infer(sCoreProcArgs* inferArgs_){}

};

struct sEngineProcArgs {
	sCore* core;
	sCoreProcArgs* coreProcArgs;

	sEngineProcArgs() {
		coreProcArgs = new sCoreProcArgs();
	}
};
