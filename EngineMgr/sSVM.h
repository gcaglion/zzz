#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sSVMparms.h"

struct sSVM : sCfgObj {
	EXPORT sSVM(sCfgObjParmsDef, sCoreLayout* layout_, sSVMparms* SVMparms_);
	EXPORT ~sSVM();
};