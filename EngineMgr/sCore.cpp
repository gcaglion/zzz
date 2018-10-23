#include "sCore.h"

sCore::sCore(sCfgObjParmsDef, sCoreLayout* layout_) : sCfgObj(sCfgObjParmsVal) {
	layout=layout_;

	//-- 1. get Parameters
	safespawn(persistor, newsname("%s_Persistor", name->base), defaultdbg, cfg, "Persistor");

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;	
	
}
sCore::~sCore() {}

