#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sGAparms.h"

struct sGA : sCfgObj {
	EXPORT sGA(sCfgObjParmsDef, sCoreLayout* layout_, sGAparms* GAparms_);
	EXPORT ~sGA();
};