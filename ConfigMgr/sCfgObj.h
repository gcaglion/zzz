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

	sCfgObj(sCfgObjParmsDef) : sObj(sObjParmsVal) {

		cfg=cfg_;
		safecall(cfg, setKey, keyDesc_);
		cfgKey=cfg->currentKey;
	}

	~sCfgObj(){}

};

/*
==============================================================================

//-- 1. get Parameters
//-- 2. do stuff and spawn sub-Keys
//-- 3. Restore currentKey
cfg->currentKey=cfg->bkpKey;

==============================================================================

*/