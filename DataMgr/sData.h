#pragma once
#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfg.h"
#include "DataSet.h"


//-- Actions on data
#define TRAIN 0
#define TEST  1
#define VALID 2

struct sDataShape : sCfgObj {

	//-- basic properties
	int sampleLen;
	int predictionLen;
	int featuresCnt;

	//-- these are set within Cores
	int inputCnt;
	int outputCnt;

	EXPORT sDataShape(sCfgObjParmsDef, int sampleLen_, int predictionLen_, int featuresCnt_);
	EXPORT sDataShape(sCfgObjParmsDef);
	EXPORT ~sDataShape();

};

struct sData : sCfgObj {

	//-- shape
	sDataShape* shape=nullptr;

	//-- actions
	Bool ActionDo[3];
	char ActionDesc[3][XMLKEY_NAME_MAXLEN]={ "Train","Test","Validation" };

	//-- DataSets (each include its own source TimeSerie)
	sDataSet* ds[3];

	EXPORT sData(sCfgObjParmsDef, sDataShape* shape_, Bool doTrain, Bool doTest, Bool doValidation);
	EXPORT sData(sCfgObjParmsDef);
	EXPORT ~sData();

};