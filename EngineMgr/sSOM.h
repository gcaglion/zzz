#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sSOMparms.h"

struct sSOM : sCfgObj {
	EXPORT sSOM(sCfgObjParmsDef, sCoreLayout* layout_, sSOMparms* SOMparms_);
	EXPORT ~sSOM();
};