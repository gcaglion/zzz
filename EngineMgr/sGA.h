#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sGAparms.h"

struct sGA : sCore {
	EXPORT sGA(sCfgObjParmsDef, sCoreLayout* layout_, sGAparms* GAparms_, sCoreLogger* persistor_);
	EXPORT ~sGA();
};