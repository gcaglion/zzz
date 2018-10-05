#include "sCore.h"

sCore::sCore(sCfgObjParmsDef, sCoreLayout* layout_) :sCfgObj(sObjParmsVal, nullptr, nullptr) {
	layout=layout_;
}
sCore::sCore(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	//-- 1. get Parameters
	//-- 2. do stuff and spawn sub-Keys
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sCore::~sCore(){}