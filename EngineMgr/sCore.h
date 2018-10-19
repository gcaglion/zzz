#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/sDataShape.h"
#include "../DataMgr/sDataSet.h"
#include "sCoreLayout.h"
#include "sCoreParms.h"
#include "sCoreLogger.h"
#include "Core_enums.h"

struct sCoreTrainArgs {
	sDataSet* ds;
	int pid;
	int tid;
	int testid;
	int screenLine;
};

struct sCore : sCfgObj {

	//int pid, tid;

	sCoreParms* parms;
	sCoreLayout* layout;
	sCoreLogger* persistor;
	sCoreTrainArgs* trainArgs;
	sCoreTrainArgs* inferArgs;

	EXPORT sCore(sCfgObjParmsDef, sCoreLayout* layout_);
	EXPORT ~sCore();

	//-- properties and methods common for all core subclasses
	int mseCnt;		// replaces ActualEpochs
	numtype* mseT;	// Training mean squared error, array indexed by epoch, always on host
	numtype* mseV;	// Validation mean squared error, array indexed by epoch, always on host
	//--

	//-- methods to be implemented indipendently by each subclass (sNN, sGA, ...)
	virtual void train(sCoreTrainArgs* trainArgs_){}
	virtual void infer(sCoreTrainArgs* inferArgs_){}

};

struct sEngineTrainArgs {
	sCore* core;
	sCoreTrainArgs* coreTrainArgs;

	sEngineTrainArgs() {
		coreTrainArgs = new sCoreTrainArgs();
	}
};
