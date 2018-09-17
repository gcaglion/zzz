#include "Forecaster.h"

sForecaster::sForecaster(sCfgObjParmsDef, sData* forecastData_) : sCfgObj(sCfgObjParmsVal) {

	data = forecastData_;

}

sForecaster::sForecaster(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	//-- 2. do stuff and spawn sub-Keys
	safespawn(data, newsname("Data"), defaultdbg, cfg, "Data");
	//-- 3. Restore currentKey
	cfg->currentKey=bkpKey;

}

sForecaster::~sForecaster(){}