#include "sEngine.h"

void sEngine::mallocs() {
	coreType = (int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	coreThreadId = (int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	coreLayer = (int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	core = (sCore**)malloc(MAX_ENGINE_CORES*sizeof(sCore*));
}
sEngine::sEngine(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	mallocs();
	pid=GetCurrentProcessId();
	safecall(cfgKey, getParm, &coresCnt, "CoresCount");
}
sEngine::sEngine(sObjParmsDef, sLogger* persistor_, int clientPid_, int savedEnginePid_) : sCfgObj(sObjParmsVal, nullptr, "") {
	mallocs();
	pid=clientPid_;
	safecall(persistor_, loadEngineCoresInfo, savedEnginePid_, &coresCnt, &coreType, &coreThreadId, &coreLayer);
}
sEngine::~sEngine(){
	free(coreType);
	free(coreThreadId);
	free(coreLayer);
	free(core);
}

void sEngine::infer(int simulationId_, int seqId_, sTS2* inferTS_, int savedEnginePid_) {
	if (savedEnginePid_>0) {
		//-- load cores
		for (int c=0; c<coresCnt; c++) {

		}
	}
}

void sEngine::commit() {
	for (int c=0; c<coresCnt; c++) {
		safecall(core[c]->persistor, commit);
	}
}

