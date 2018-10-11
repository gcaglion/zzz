#pragma once
#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfg.h"
#include "DataSet.h"
#include "DataShape.h"


struct sData : sCfgObj {

	//-- shape
	sDataShape* shape;

	//-- DataSets (each include its own source TimeSerie)
	sDataSet* trainDS;
	sDataSet* testDS;
	sDataSet* validDS;
	bool doTraining, doInference, doValidation;

	EXPORT sData(sCfgObjParmsDef, sDataShape* shape_, sDataSet* trainDS_=nullptr, sDataSet* testDS_=nullptr, sDataSet* validDS_=nullptr);
	EXPORT sData(sCfgObjParmsDef);
	EXPORT ~sData();

};