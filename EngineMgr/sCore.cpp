#include "sCore.h"

sCore::sCore(sCfgObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_) : sCfgObj(sCfgObjParmsVal) {
	layout=layout_;
	persistor=persistor_;
	safespawn(Alg, newsname("%s_Algebra", name->base), defaultdbg);
}
sCore::sCore(sCfgObjParmsDef, sCoreLayout* layout_) : sCfgObj(sCfgObjParmsVal) {
	layout=layout_;
	safespawn(Alg, newsname("%s_Algebra", name->base), defaultdbg);

	//-- 1. get Parameters
	safespawn(persistor, newsname("%s_Persistor", name->base), defaultdbg, cfg, "Persistor");
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sCore::~sCore() {}
