#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/sDataShape.h"
#include "../EngineMgr/sEngine.h"

struct sForecaster : sCfgObj {

	int pid;
	
	bool doTraining, doInference, doValidation;
	int enginePid;

	sLogger* persistor;

	sDataShape* shape;	//-- common across datasets
	int featuresCnt;	//-- specific for the engine. depends on datasets used
	sDataSet* trainDS;
	sDataSet* inferDS;
	sDataSet* validDS;

	sEngine* engine;

	EXPORT sForecaster(sCfgObjParmsDef, sDataShape* shape_, bool doTraining_, sDataSet* trainDS_, bool doInference_, sDataSet* inferDS_, bool doValidation_, sDataSet* validDS_, int enginePid_, sEngine* engine_);
	EXPORT sForecaster(sCfgObjParmsDef);
	EXPORT ~sForecaster();

	void getForecast(char* trainDate0_=nullptr, char* testDate0_=nullptr, char* validDate0_=nullptr);	//-- if any of these is nullptr, then we use date0 from cfg dedicated section
};


