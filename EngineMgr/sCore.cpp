#include "sCore.h"

sCore::sCore(sCfgObjParmsDef, int inputCnt_, int outputCnt_, sCoreLogger* persistor_) : sCfgObj(sCfgObjParmsVal) {
	inputCnt=inputCnt_; outputCnt=outputCnt_;
	persistor=persistor_;
	procArgs=new sCoreProcArgs();
	safespawn(Alg, newsname("%s_Algebra", name->base), defaultdbg);
}
sCore::sCore(sCfgObjParmsDef, int inputCnt_, int outputCnt_) : sCfgObj(sCfgObjParmsVal) {
	inputCnt=inputCnt_; outputCnt=outputCnt_;
	procArgs=new sCoreProcArgs();
	safespawn(Alg, newsname("%s_Algebra", name->base), defaultdbg);

	//-- 1. get Parameters
	safespawn(persistor, newsname("%s_Persistor", name->base), defaultdbg, cfg, "Persistor");
	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sCore::~sCore() {
	delete procArgs;
}
