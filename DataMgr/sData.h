#pragma once
#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfg.h"
#include "sDataSet.h"
#include "sDataShape.h"


struct sData : sCfgObj {

	//-- shape
	sDataShape* shape;

	//-- DataSets (each include its own source TimeSerie)
	sDataSet* trainDS;
	sDataSet* testDS;
	sDataSet* validDS;
	bool doTraining, doInference, doValidation;
	int enginePid;	//-- used to load the engine before inferencing

	EXPORT sData(sObjParmsDef, sDataShape* shape_, sDataSet* trainDS_=nullptr, sDataSet* testDS_=nullptr, sDataSet* validDS_=nullptr);
	EXPORT sData(sCfgObjParmsDef);
	EXPORT ~sData();

};