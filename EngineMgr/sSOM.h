#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sSOMparms.h"

struct sSOM : sCore {
	EXPORT sSOM(sCfgObjParmsDef, sCoreLayout* layout_, sSOMparms* SOMparms_);
	EXPORT ~sSOM();
};