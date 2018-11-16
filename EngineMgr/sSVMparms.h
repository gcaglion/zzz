#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../EngineMgr/sCoreParms.h"
#include "sGAenums.h"

struct sSVMparms : sCoreParms {

	//-- placeholders
	int parm1=0;
	int parm2=0;

	EXPORT sSVMparms(sCfgObjParmsDef);
	EXPORT sSVMparms(sObjParmsDef, sLogger* persistor_, int loadingPid_, int loadingTid_);
	EXPORT ~sSVMparms();

	//-- local implementations of virtual functions defined in sCoreParms
	EXPORT void setScaleMinMax();
	EXPORT void save(sLogger* persistor_, int pid_, int tid_);

};