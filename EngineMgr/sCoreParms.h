#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../Logger/sLogger.h"

#define CORE_MAX_INTERNAL_LEVELS 128

struct sCoreParms : sCfgObj {

	//-- scaling parameters are common across core types. 
	int levelsCnt;
	//-- depending on the specifig core type, there can be multiple scaling parms (e.g. NN can have a different one for each level)
	float scaleMin[CORE_MAX_INTERNAL_LEVELS];
	float scaleMax[CORE_MAX_INTERNAL_LEVELS];

	EXPORT sCoreParms(sObjParmsDef, sLogger* persistor_, int loadingPid_);
	EXPORT sCoreParms(sCfgObjParmsDef);
	EXPORT ~sCoreParms();

	virtual void setScaleMinMax()=0;
	virtual void save(int pid, int tid)=0;
	virtual void load(int pid, int tid)=0;

};