#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"

struct sCoreParms : sCfgObj {

	//-- scaling parameters are common across core types. 
	int levelsCnt;
	//-- depending on the specifig core type, there can be multiple scaling parms (e.g. NN can have a different one for each level)
	float* scaleMin; 
	float* scaleMax;

	EXPORT sCoreParms(sCfgObjParmsDef);
	EXPORT ~sCoreParms();

	virtual void setScaleMinMax(){}
};