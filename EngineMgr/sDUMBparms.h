#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../EngineMgr/sCoreParms.h"
//#include "sDUMBenums.h"

#define MAX_LEVELS 128

struct sDUMBparms : sCoreParms {
	sDUMBparms(sCfgObjParmsDef);
	sDUMBparms(sObjParmsDef, sLogger* persistor_, int loadingPid_);
	~sDUMBparms();

	//-- local implementations of virtual functions defined in sCoreParms
	EXPORT void setScaleMinMax();
	EXPORT void save(int pid, int tid);
	EXPORT void load(int pid, int tid);

};