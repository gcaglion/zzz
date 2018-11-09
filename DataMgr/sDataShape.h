#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"

struct sDataShape : sCfgObj {

	//-- basic properties
	int sampleLen;
	int predictionLen;
	//int featuresCnt;

	EXPORT sDataShape(sObjParmsDef, int sampleLen_, int predictionLen_);
	EXPORT sDataShape(sCfgObjParmsDef);
	EXPORT ~sDataShape();

};

