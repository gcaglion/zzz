#pragma once
#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfg.h"
#include "DataSet.h"
#include "DataShape.h"


//-- Actions on data
#define TRAIN 0
#define TEST  1
#define VALID 2

struct sData : sCfgObj {

	//-- shape
	sDataShape* shape;

	//-- actions
	bool ActionDo[3];
	char ActionDesc[3][XMLKEY_NAME_MAXLEN]={ "Train","Test","Validation" };

	//-- DataSets (each include its own source TimeSerie)
	sDataSet* ds[3];

	EXPORT sData(sCfgObjParmsDef, sDataShape* shape_, bool doTrain, bool doTest, bool doValidation);
	EXPORT sData(sCfgObjParmsDef);
	EXPORT ~sData();

};