#pragma once
#include "../common.h"
#include "../BaseObj/sObj.h"
#include "sCfg.h"

#define sCfgObjParmsDef sObjParmsDef, sCfg* cfg_, const char* keyDesc_
#define sCfgObjParmsVal sObjParmsVal, cfg_, keyDesc_

struct sCfgObj : sObj {

	sCfg* cfg;
	sCfgKey* cfgKey;
	sCfgKey* bkpKey;

	EXPORT sCfgObj(sCfgObjParmsDef);
	EXPORT ~sCfgObj();

};

/*
==============================================================================

//-- 1. get Parameters
//-- 2. do stuff and spawn sub-Keys
//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
cfg->currentKey=bkpKey;

==============================================================================

*/