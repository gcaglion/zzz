#include "Forecaster.h"

sForecaster::sForecaster(sCfgObjParmsDef, sData* data_, /*sEngine* engine_, */sLogger* persistor_) : sCfgObj(sCfgObjParmsVal) {

	data = data_;

}

sForecaster::sForecaster(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	//-- 2. do stuff and spawn sub-Keys
	safespawn(false, data, newsname("Data"), defaultdbg, cfg, "Data");

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}

sForecaster::~sForecaster(){}