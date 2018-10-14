#include "sCore.h"

sCore::sCore(sCfgObjParmsDef, sCoreLayout* layout_) : sCfgObj(sCfgObjParmsVal) {
	layout=layout_; 

	//-- 1. get Parameters

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;	


}
sCore::~sCore() {
}

