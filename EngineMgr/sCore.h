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
	int tsFeaturesCnt;
	int selectedFeaturesCnt;
	int* selectedFeature;
	int predictionLen;
	numtype* targetBFS;
	numtype* predictionBFS;
	numtype* targetSBF;
	numtype* predictionSBF;
};

struct sCore : sCfgObj {

	int _batchCnt;
	int _batchSize;

	sCoreParms* parms;
	sCoreLayout* layout;
	sCoreLogger* persistor;
	sCoreProcArgs* procArgs;

	EXPORT sCore(sCfgObjParmsDef, sCoreLayout* layout_);
	EXPORT ~sCore();

	//-- methods to be implemented indipendently by each subclass (sNN, sGA, ...)
	virtual void setLayout(int batchSize_)=0;
	virtual void mallocLayout()=0;
	virtual void train(sCoreProcArgs* procArgs_)=0;	
	virtual void singleInfer(numtype* singleSampleSBF, numtype* singleTargetSBF, numtype** singlePredictionSBF)=0;	//-- should set singlePredictionSBF

	void inferNEW(int samplesCnt_, int sampleLen_, int predictionLen_, int featuresCnt_, numtype* INsampleSBF, numtype* INtargetSBF, numtype* OUTpredictionSBF);


};

struct sEngineProcArgs {
	sCore* core;
	sCoreProcArgs* coreProcArgs;

	sEngineProcArgs() {
		coreProcArgs = new sCoreProcArgs();
	}
};
