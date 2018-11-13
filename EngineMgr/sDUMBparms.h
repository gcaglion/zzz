#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../EngineMgr/sCoreParms.h"
//#include "sDUMBenums.h"

#define MAX_LEVELS 128

struct sDUMBparms : sCoreParms {
	sDUMBparms(sCfgObjParmsDef);
	sDUMBparms(sObjParmsDef);
	~sDUMBparms();

	EXPORT void setScaleMinMax();
};