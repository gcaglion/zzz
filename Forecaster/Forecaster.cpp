#include "Forecaster.h"

sForecaster::sForecaster(sCfgObjParmsDef, sData* forecastData_) : sCfgObj(sCfgObjParmsVal) {

	data = forecastData_;

}

sForecaster::sForecaster(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 0. Backup currentKey
	sCfgKey* bkpKey=cfg->currentKey;
	//-- 1. set Key 
	safecall(cfg, setKey, keyDesc_);
	//-- 2. get Parameters
	//-- 3. spawn sub-Keys
	data=new sData(this, newsname("Data"), defaultdbg, cfg, "Data");

	//_spawn(__func__, &data, newsname("Forecaster.Data"), defaultdbg, cfg, "Data");
	//safespawn(data, newsname("Forecaster.Data"), defaultdbg, cfg, "Data");
	//-- 4. Restore currentKey
	cfg->currentKey=bkpKey;

}

sForecaster::~sForecaster(){}