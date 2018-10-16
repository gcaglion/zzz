#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../EngineMgr/sCoreParms.h"
#include "sGAenums.h"

#define MAX_LEVELS 128

struct sSVMparms : sCoreParms {
	sSVMparms(sCfgObjParmsDef);
	sSVMparms(sObjParmsDef);
	~sSVMparms();

	EXPORT void setScaleMinMax();
};