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
	numtype mseR;
	int internalsCnt;
	//-- infer-specific
	int npid;
	int ntid;
	bool loadImage;
};

struct sCore : sCfgObj {

	int _batchCnt;
	int _batchSize;

	sCoreParms* parms;
	sCoreLayout* layout;
	sCoreLogger* persistor;
	sCoreProcArgs* procArgs;

	//-- MyAlgebra common structures
	sAlgebra* Alg;

	EXPORT sCore(sCfgObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sCoreLogger* persistor_);
	EXPORT sCore(sCfgObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_);
	EXPORT ~sCore();




	//-- methods to be implemented indipendently by each subclass(sNN, sGA, ...)
	
	//-- Internal layout
	virtual void setLayout(int batchSize_)=0;
	virtual void mallocLayout()=0;
	virtual void train(sCoreProcArgs* procArgs_)=0;
	virtual void infer(sCoreProcArgs* procArgs_)=0;
	//virtual void infer(int samplesCnt_, int sampleLen_, int predictionLen_, int featuresCnt_, numtype* INsampleSBF, numtype* INtargetSBF, numtype* OUTpredictionSBF);	// NOT abstract
	//virtual void singleInfer(int sampleLen_, int sampleFeaturesCnt_, int batchSamplesCnt_, numtype* singleSampleBF, numtype* singleTargetBF, numtype** singlePredictionBF)=0;
	//-- I/O	
	virtual void saveImage(int pid, int tid, int epoch)=0;
	virtual void loadImage(int pid, int tid, int epoch)=0;



};

struct sEngineProcArgs {
	sCore* core;
	sCoreProcArgs* coreProcArgs;

	sEngineProcArgs() {
		coreProcArgs = new sCoreProcArgs();
	}
};
