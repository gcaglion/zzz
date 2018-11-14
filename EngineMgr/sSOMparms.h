#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../EngineMgr/sCoreParms.h"
#include "sGAenums.h"

#define MAX_LEVELS 128

struct sSOMparms : sCoreParms {
	
	//-- placeholders
	int parm1=0;
	int parm2=0;

	EXPORT sSOMparms(sCfgObjParmsDef);
	EXPORT sSOMparms(sObjParmsDef, sLogger* persistor_, int loadingPid_, int loadingTid_);
	EXPORT ~sSOMparms();

	//-- local implementations of virtual functions defined in sCoreParms
	EXPORT void setScaleMinMax();
	EXPORT void save(sLogger* persistor_, int pid_, int tid_);

};