#include "sCore.h"

sCore::sCore(sCfgObjParmsDef, sCoreLayout* layout_) : sCfgObj(sCfgObjParmsVal) {
	layout=layout_;

	pid=GetCurrentProcessId();
	tid=GetCurrentThreadId();

	//-- 1. get Parameters
	safespawn(persistor, newsname("%s_Persistor", name->base), defaultdbg, cfg, "Persistor");

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;	
	
}
sCore::~sCore() {}

LRESULT sCore::trainThread(LPVOID parm, sDataSet* trainDS_) {
	sCore* pObject=reinterpret_cast<sCore*>(parm);
	pObject->train(trainDS_);
	return 0;
}
LRESULT sCore::inferThread(LPVOID parm, sDataSet* trainDS_) {
	sCore* pObject=reinterpret_cast<sCore*>(parm);
	pObject->train(trainDS_);
	return 0;
}
