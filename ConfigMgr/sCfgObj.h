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

		//-- cfg_ and keyDesc_ are both nullptr when we call a sCfgObj from a direct contructor (i.e. without config information)
		if (cfg_!=nullptr) {
			cfg=cfg_;
			//-- 0. backup cfg currentKey into Object bkpKey
			bkpKey=cfg->currentKey;
			//-- 1. set cfg currentKey
			safecall(cfg, setKey, keyDesc_);
			//-- 2. set Object cfgKey
			cfgKey=cfg->currentKey;
			//-- 3. restore cfg currentKey
			//cfg->currentKey=bkpKey;
		}
	}

	~sCfgObj(){}

};

/*
==============================================================================

//-- 1. get Parameters
//-- 2. do stuff and spawn sub-Keys
//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
cfg->currentKey=bkpKey;

==============================================================================

*/