#include "sCore.h"

sCore::sCore(sCfgObjParmsDef, sCoreLayout* layout_, sDataShape* dataShape_) : sCfgObj(sCfgObjParmsVal) {
	layout=layout_; baseDataShape=dataShape_;

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &type, "Type");

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;	


}
sCore::~sCore() {
}

