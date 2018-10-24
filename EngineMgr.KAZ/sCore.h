#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/sDataShape.h"
#include "../DataMgr/sDataSet.h"
#include "sCoreLayout.h"
#include "sCoreParms.h"
#include "sCoreLogger.h"
#include "Core_enums.h"

struct sCore : sCfgObj {

	int pid, tid;

	sCoreParms* parms;
	sCoreLayout* layout;
	sCoreLogger* persistor;
	sDataSet* inputDS;
	sDataSet* outputDS;

	EXPORT sCore(sCfgObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_);
	EXPORT ~sCore();

	//-- properties and methods common for all core subclasses
	int mseCnt;		// replaces ActualEpochs
	numtype* mseT;	// Training mean squared error, array indexed by epoch, always on host
	numtype* mseV;	// Validation mean squared error, array indexed by epoch, always on host


	//-- methods to be implemented indipendently by each subclass (sNN, sGA, ...)
	void sCore::loadInput(sDataSet* inputDS_) { inputDS=inputDS_; }
	virtual void train(sDataSet* ds_){}
	virtual void infer(sDataSet* ds_){}

};
