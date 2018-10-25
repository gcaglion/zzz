#include "Forecaster.h"

sForecaster::sForecaster(sCfgObjParmsDef, sData* data_, sEngine* engine_) : sCfgObj(sCfgObjParmsVal) {
	data = data_;
	engine = engine_;
}
sForecaster::sForecaster(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	
	//-- 2. do stuff and spawn sub-Keys
	safespawn(data, newsname("Data"), defaultdbg, cfg, "Data");
	safespawn(engine, newsname("Engine"), defaultdbg, cfg, "Engine", data->shape->sampleLen*data->shape->featuresCnt, data->shape->predictionLen*data->shape->featuresCnt);

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sForecaster::~sForecaster(){}

void sForecaster::getForecast(char* trainDate0_, char* testDate0_, char* validDate0_) {

}

