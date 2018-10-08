#include "Forecaster.h"

sForecaster::sForecaster(sCfgObjParmsDef, sData* data_, sEngine* engine_, sLogger* persistor_) : sCfgObj(sCfgObjParmsVal) {

	data = data_;
	engine = engine_;
	persistor = persistor_;
}
sForecaster::sForecaster(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	//-- 2. do stuff and spawn sub-Keys
	safespawn(false, data, newsname("Data"), defaultdbg, cfg, "Data");
	safespawn(false, engine, newsname("Engine"), defaultdbg, cfg, "Engine", data->shape);
	safespawn(false, persistor, newsname("Persistor"), defaultdbg, cfg, "Persistor");

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sForecaster::~sForecaster(){}

void sForecaster::getForecast() {

}

