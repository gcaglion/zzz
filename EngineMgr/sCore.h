#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../Algebra/Algebra.h"
#include "sCoreLayout.h"
#include "sCoreParms.h"
#include "sCoreLogger.h"
#include "Core_enums.h"

//-- properties in sCoreProcArgs are common across core types
struct sCoreProcArgs {
	//-- common across train and infer
	std::exception_ptr excp;
	int samplesCnt;
	int batchCnt;
	int batchSize;
	numtype* sample;
	numtype* target;
	numtype* prediction;
	int pid;
	int tid;
	int testid;
	int screenLine;
	//-- train-specific
	int mseCnt;
	numtype* mseT;
	numtype* mseV;
	int internalsCnt;
	int* duration;
	//-- infer-specific
	int npid;
	int ntid;
	numtype mseR;
	bool quitAfterBreak;
};

struct sCore : sCfgObj {

	sCoreParms* parms;
	//sCoreLayout* layout;
	int inputCnt;
	int outputCnt;
	sCoreLogger* persistor;
	sCoreProcArgs* procArgs;

	//-- MyAlgebra common structures
	sAlgebra* Alg;

	EXPORT sCore(sCfgObjParmsDef, int inputCnt_, int outputCnt_, sCoreLogger* persistor_);
	EXPORT sCore(sCfgObjParmsDef, int inputCnt_, int outputCnt_);
	EXPORT ~sCore();

	//-- methods to be implemented indipendently by each subclass(sNN, sGA, ...)
	
	//-- Internal layout
	virtual void setLayout()=0;
	//virtual void mallocLayout()=0;
	virtual void train()=0;
	virtual void infer()=0;
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
	~sEngineProcArgs() {
		delete coreProcArgs;
	}
};
