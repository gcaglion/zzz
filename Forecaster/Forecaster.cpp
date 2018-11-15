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

	//-- data shape is common across datasets and engine
	safespawn(shape, newsname("DataShape"), defaultdbg, cfg, "DataShape");

	//-- forecaster-level persistor. this is used, among other things, to spawn engine by pid
	safespawn(persistor, newsname("ForecasterPersistor"), defaultdbg, cfg, "Persistor");

	if (doInference) {
		safespawn(inferDS, newsname("InferDataSet"), defaultdbg, cfg, "Test/DataSet", shape->sampleLen, shape->predictionLen);
	}
	if (doTraining) {
		safespawn(trainDS, newsname("TrainDataSet"), defaultdbg, cfg, "Train/DataSet", shape->sampleLen, shape->predictionLen);
	}

	//-- Engine featuresCnt is determined by trainig Dataset if we do training: by inference dataset otherwise
	featuresCnt=(doTraining) ? trainDS->selectedFeaturesCnt : inferDS->selectedFeaturesCnt;

	//-- 3. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sForecaster::~sForecaster(){}

void sForecaster::getForecast(char* trainDate0_, char* testDate0_, char* validDate0_) {

}

void sForecaster::setEngine(){

	//-- 1. spawn engine from either xml or db
	if (doTraining) {
		//-- spawn engine standard way
		safespawn(engine, newsname("Engine"), defaultdbg, cfg, "/Forecaster/Engine", shape->sampleLen*featuresCnt, shape->predictionLen*featuresCnt);

	} else {
		if (doInference) {
			safecall(cfgKey, getParm, &enginePid, "InferFromPid");
			//-- spawn engine from forecaster->persistor with pid
			safespawn(engine, newsname("Engine"), defaultdbg, cfg, "/Forecaster/Engine", shape->sampleLen*featuresCnt, shape->predictionLen*featuresCnt, persistor, enginePid);
		}
	}


}
void sForecaster::trainBlock(int simulId, char* startDate) {
	if (doTraining) {
		//-- 6.1.1. set date0 in trainDS->TimeSerie, and load it
		safecall(trainDS->sourceTS, load, TARGET, BASE, startDate);
		//-- 6.1.2. spawn engine from config file

		//-- 6.1.3. do training (also populates datasets)
		safecall(engine, train, simulId, trainDS);
		//-- 6.1.4. persist MSE logs
		safecall(engine, saveMSE);
		//-- 6.1.5 persist Core logs
		safecall(engine, saveImage);
		//-- 6.1.6 persist Engine Info
		safecall(engine, saveInfo);
	}
}
void sForecaster::inferBlock(int simulId, char* startDate) {
	if (doInference) {
		//-- 6.2.1. set date0 in testDS->TimeSerie, and load it
		safecall(inferDS->sourceTS, load, TARGET, BASE, startDate);

		//-- 6.2.2. do inference (also populates datasets)
		safecall(engine, infer, simulId, inferDS);
		//-- 6.2.3. persist Run logs
		safecall(engine, saveRun);
	}
}
