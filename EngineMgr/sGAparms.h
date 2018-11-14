#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../EngineMgr/sCoreParms.h"
#include "sGAenums.h"

#define MAX_LEVELS 128

struct sGAparms : sCoreParms {
	sGAparms(sCfgObjParmsDef);
	sGAparms(sObjParmsDef, sLogger* persistor_, int loadingPid_);
	~sGAparms();

	//-- local implementations of virtual functions defined in sCoreParms
	EXPORT void setScaleMinMax();
	EXPORT void save(int pid, int tid);
	EXPORT void load(int pid, int tid);

};