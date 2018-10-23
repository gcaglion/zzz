#include "Forecaster.h"

sForecaster::sForecaster(sObjParmsDef, sDataShape* shape_, bool doTrain_, sDataSet* trainDS_, bool doInfer_, sDataSet* inferDS_, bool doValid_, sDataSet* validDS_, sEngine* engine_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	shape=shape_;
	doTrain=doTrain_; trainDS=trainDS_; 	
	doInfer=doInfer_; inferDS=inferDS_; 
	doValid=doValid_; validDS=validDS_;
	engine = engine_;
}
sForecaster::sForecaster(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	doTrain=false; doValid=false; doInfer=false;

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &doTrain, "DoTraining");
	safecall(cfgKey, getParm, &doInfer, "DoInference");
	safecall(cfgKey, getParm, &doValid, "DoValidation");
	//-- 2. do stuff and spawn sub-Keys
	safespawn(shape, newsname("Shape"), defaultdbg, cfg, "Shape");
	if(doTrain) safespawn(trainDS, newsname("TrainDataSet"), defaultdbg, cfg, "TrainingDataSet");
	if(doInfer) safespawn(inferDS, newsname("InferDataSet"), defaultdbg, cfg, "InferenceDataSet");
	if(doValid) safespawn(validDS, newsname("ValidationDataSet"), defaultdbg, cfg, "ValidationDataSet");
	safespawn(engine, newsname("Engine"), defaultdbg, cfg, "Engine", shape->sampleLen*shape->featuresCnt, shape->targetLen*shape->featuresCnt);

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sForecaster::~sForecaster(){}

void sForecaster::getForecast(char* trainDate0_, char* testDate0_, char* validDate0_) {

}

