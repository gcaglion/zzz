#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../EngineMgr/sCoreParms.h"
#include "sGAenums.h"

#define MAX_LEVELS 128

struct sGAparms : sCoreParms {
	
	EXPORT sGAparms(sCfgObjParmsDef);
	EXPORT sGAparms(sObjParmsDef, sLogger* persistor_, int loadingPid_);
	EXPORT ~sGAparms();

	//-- local implementations of virtual functions defined in sCoreParms
	EXPORT void setScaleMinMax();
	EXPORT void save(int pid, int tid);
	EXPORT void load(int pid, int tid);

};