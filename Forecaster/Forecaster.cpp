#include "Forecaster.h"

sForecaster::sForecaster(sCfgObjParmsDef, sDataShape* shape_, bool doTraining_, sDataSet* trainDS_, bool doInference_, sDataSet* inferDS_, bool doValidation_, sDataSet* validDS_, int enginePid_, sEngine* engine_) : sCfgObj(sCfgObjParmsVal) {
	pid=GetCurrentProcessId();
	shape=shape_;
	doTraining=doTraining_; trainDS=trainDS_; 
	doInference=doInference_; inferDS=inferDS_; 
	doValidation=doValidation_; validDS=validDS_;
	enginePid=enginePid_; engine = engine_;
}
sForecaster::sForecaster(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	pid=GetCurrentProcessId();

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &doTraining, "DoTraining");
	safecall(cfgKey, getParm, &doInference, "DoInference");
	safecall(cfgKey, getParm, &doValidation, "DoValidation");
	if (doInference) safecall(cfgKey, getParm, &enginePid, "InferFromPid");

	//-- data shape is common across datasets and engine
	safespawn(shape, newsname("DataShape"), defaultdbg, cfg, "DataShape");


	if (doInference) {
		safespawn(inferDS, newsname("InferDataSet"), defaultdbg, cfg, "Test/DataSet", shape->sampleLen, shape->predictionLen);
	}
	if (doTraining) {
		safespawn(trainDS, newsname("TrainDataSet"), defaultdbg, cfg, "Train/DataSet", shape->sampleLen, shape->predictionLen);
	} else {
		//-- load pid
	}

	//-- Engine featuresCnt is determined by trainig Dataset if we do training: by inference dataset otherwise
	featuresCnt=(doTraining) ? trainDS->selectedFeaturesCnt : inferDS->selectedFeaturesCnt;
	if (doTraining) {
		safespawn(engine, newsname("Engine"), defaultdbg, cfg, "Engine", shape->sampleLen*featuresCnt, shape->predictionLen*featuresCnt);
	} else {
		if (doInference) {
			//--load pid
			safecall(cfgKey, getParm, &enginePid, "InferFromPid");
			safespawn(engine, newsname("Engine"), defaultdbg, cfg, "Engine", shape->sampleLen*featuresCnt, shape->predictionLen*featuresCnt, enginePid);
		} else {
			return;
		}
	}

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sForecaster::~sForecaster(){}

void sForecaster::getForecast(char* trainDate0_, char* testDate0_, char* validDate0_) {

}

