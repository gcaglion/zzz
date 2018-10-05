#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "Core_enums.h"

struct sCore : sCfgObj {

	sCoreLayout* layout;

	EXPORT sCore(sCfgObjParmsDef, sCoreLayout* layout_);
	EXPORT sCore(sCfgObjParmsDef);

};