#include "Forecaster.h"

sForecaster::sForecaster(sCfgObjParmsDef, sData* data_, sEngine* engine_) : sCfgObj(sCfgObjParmsVal) {
	data = data_;
	engine = engine_;
}
sForecaster::sForecaster(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {

	//-- 1. get Parameters
	
	//-- 2. do stuff and spawn sub-Keys
	safespawn(data, newsname("Data"), defaultdbg, cfg, "Data");
	//-- Engine featuresCnt is determined by trainig Dataset if we do training: by inference dataset otherwise
	int featuresCnt=(data->doTraining) ? data->trainDS->selectedFeaturesCnt : data->testDS->selectedFeaturesCnt;
	safespawn(engine, newsname("Engine"), defaultdbg, cfg, "Engine", data->shape->sampleLen*featuresCnt, data->shape->predictionLen*featuresCnt);

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sForecaster::~sForecaster(){}

void sForecaster::getForecast(char* trainDate0_, char* testDate0_, char* validDate0_) {

}

