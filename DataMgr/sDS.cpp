#include "sDS.h"

sDS::sDS(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	safecall(cfgKey, getParm, &sampleLen, "SampleLen");
	safecall(cfgKey, getParm, &targetLen, "TargetLen");
	//--
	safecall(cfgKey, getParm, &doDump, "Dump");
	strcpy_s(dumpPath, MAX_PATH, dbg->outfilepath);
	char* _dumpPath=&dumpPath[0];
	safecall(cfgKey, getParm, &_dumpPath, "DumpPath", true);
	//--
	sTS* _ts; safespawn(_ts, newsname("%s_TimeSerie"), defaultdbg, cfg, "TimeSerie");

	cfg->currentKey=bkpKey;
}

sDS::sDS(sObjParmsDef, int fromDScnt_, sDS** fromDS_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {}
sDS::~sDS(){}