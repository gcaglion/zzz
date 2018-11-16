#include "sCfgObj.h"

sCfgObj::sCfgObj(sCfgObjParmsDef) : sObj(sObjParmsVal) {

	//-- cfg_ and keyDesc_ are both nullptr when we call a sCfgObj from a direct contructor (i.e. without config information)
	cfg=cfg_;
	if (cfg!=nullptr) {
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
sCfgObj::~sCfgObj() {
	//printf("sCfgObj Destructor called for %s\n", name->full);
}
