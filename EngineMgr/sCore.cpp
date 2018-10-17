#include "sCore.h"

sCore::sCore(sCfgObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_) : sCfgObj(sCfgObjParmsVal) {
	layout=layout_; persistor=persistor_;

	pid=GetCurrentProcessId();
	tid=GetCurrentThreadId();

	//-- 1. get Parameters

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;	
	
}
sCore::~sCore() {}
