#include "sRoot.h"

//-- constructor / destructor
sRoot::sRoot(int argc_, char* argv_[]) : sObj(nullptr, newsname("RootObj"), defaultdbg) {
	dbg->verbose=false;

	pid=GetCurrentProcessId();
//	CLoverride(argc_, argv_);
}
sRoot::~sRoot() {}

//-- core stuff
void sRoot::trainClient(const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_) {

	//-- full filenames
	char clientffname[MAX_PATH];
	char shapeffname[MAX_PATH];
	char trainffname[MAX_PATH];
	char engineffname[MAX_PATH];

	//-- client vars
	int simulationLength;
	char** simulationTrainStartDate=nullptr;
	char** simulationInferStartDate=nullptr;
	char** simulationValidStartDate=nullptr;

	char* endtimeS;
	sTimer* timer=new sTimer();

	sDataShape* shape;
	sDataSet* trainDS; sLogger* trainLog;
	sEngine* engine;
	sLogger* clientLog;


	try {
		//-- 0. set full file name for each of the input files
		getFullPath(clientXMLfile_, clientffname);
		getFullPath(shapeXMLfile_, shapeffname);
		getFullPath(trainXMLfile_, trainffname);
		getFullPath(engineXMLfile_, engineffname);

		//-- 1. load separate sCfg* for client, dataShape, trainDataset, Engine
		sCfg* clientCfg; safespawn(clientCfg, newsname("clientCfg"), defaultdbg, clientffname);
		sCfg* shapeCfg; safespawn(shapeCfg, newsname("shapeCfg"), defaultdbg, shapeffname);
		sCfg* trainCfg; safespawn(trainCfg, newsname("trainCfg"), defaultdbg, trainffname);
		sCfg* engCfg; safespawn(engCfg, newsname("engineCfg"), defaultdbg, engineffname);

		//-- 2. spawn DataShape
		safespawn(shape, newsname("TrainDataShape"), defaultdbg, shapeCfg, "/DataShape");
		//-- 3. spawn Train DataSet and its persistor
		safespawn(trainDS, newsname("TrainDataSet"), defaultdbg, trainCfg, "/DataSet", shape->sampleLen, shape->predictionLen);
		safespawn(trainLog, newsname("TrainLogger"), defaultdbg, trainCfg, "/DataSet/Persistor");
		//-- 4. spawn engine the standard way
		safespawn(engine, newsname("TrainEngine"), defaultdbg, engCfg, "/Engine", shape->sampleLen*trainDS->selectedFeaturesCnt, shape->predictionLen*trainDS->selectedFeaturesCnt);

		//=====================================================================================================================================================================
		//-- 5.1 create client persistor, if needed
		bool saveClient;
		safecall(clientCfg, setKey, "/Client");
		safecall(clientCfg->currentKey, getParm, &saveClient, "saveClient");
		safespawn(clientLog, newsname("ClientLogger"), defaultdbg, clientCfg, "Persistor");

		//-- 5.2 set simulation dates
		safecall(this, mallocSimulationDates, clientCfg, &simulationLength, &simulationTrainStartDate, &simulationInferStartDate, &simulationValidStartDate);
		//=====================================================================================================================================================================

		//-- 5. for each simulation date,
		for (int s=0; s<simulationLength; s++) {
			//-- 5.1. start timer
			timer->start();
			//-- 5.2. get simulation date for s
			safecall(clientCfg->currentKey, getParm, simulationTrainStartDate, "TrainStartDate");
			getStartDates(trainDS, simulationTrainStartDate[0], simulationLength, &simulationTrainStartDate);
			//-- set date0 in trainDS->TimeSerie, and load it
			safecall(trainDS->sourceTS, load, TARGET, BASE, simulationTrainStartDate[s]);
			//-- do training (also populates datasets)
			safecall(engine, train, s, trainDS);
			//-- persist MSE logs
			safecall(engine, saveMSE);
			//-- persist Core logs
			safecall(engine, saveImage);
			//-- persist Engine Info
			safecall(engine, saveInfo);
			//-- 5.4 Commit engine persistor data
			safecall(engine, commit);

			//-- 5.5. stop timer, and save client info
			endtimeS=timer->stop();
			safecall(clientLog, saveClientInfo, pid, s, "Root.Tester", timer->startTime, timer->elapsedTime, simulationTrainStartDate[s], simulationInferStartDate[s], simulationValidStartDate[s], true, false, false);
			//-- 5.4 Commit clientpersistor data
			safecall(clientLog, commit);

			//-- 
		}

	}
	catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}


}
void sRoot::inferClient(const char* clientXMLfile_, const char* shapeXMLfile_, const char* inferXMLfile_, const char* engineXMLfile_, int savedEnginePid_) {

	//-- full filenames
	char clientffname[MAX_PATH];
	char shapeffname[MAX_PATH];
	char inferffname[MAX_PATH];
	char engineffname[MAX_PATH];

	//-- client vars
	int simulationLength;
	char** simulationTrainStartDate=nullptr;
	char** simulationInferStartDate=nullptr;
	char** simulationValidStartDate=nullptr;

	char* endtimeS;
	sTimer* timer=new sTimer();

	sDataShape* shape;
	sDataSet* inferDS; sLogger* inferLog;
	sEngine* engine;
	sLogger* clientLog;
	sLogger* engLog;

	try {
		//-- 0. set full file name for each of the input files
		getFullPath(clientXMLfile_, clientffname);
		getFullPath(shapeXMLfile_, shapeffname);
		getFullPath(inferXMLfile_, inferffname);
		getFullPath(engineXMLfile_, engineffname);

		//-- 1. load separate sCfg* for client, dataShape, inferDataset, Engine
		sCfg* clientCfg; safespawn(clientCfg, newsname("clientCfg"), defaultdbg, clientffname);
		sCfg* shapeCfg; safespawn(shapeCfg, newsname("shapeCfg"), defaultdbg, shapeffname);
		sCfg* inferCfg; safespawn(inferCfg, newsname("inferCfg"), defaultdbg, inferffname);
		sCfg* engCfg; safespawn(engCfg, newsname("engineCfg"), defaultdbg, engineffname);

		//-- 2. spawn DataShape
		safespawn(shape, newsname("inferDataShape"), defaultdbg, shapeCfg, "/DataShape");
		//-- 3. spawn infer DataSet and its persistor
		safespawn(inferDS, newsname("inferDataSet"), defaultdbg, inferCfg, "/DataSet", shape->sampleLen, shape->predictionLen);
		safespawn(inferLog, newsname("inferLogger"), defaultdbg, inferCfg, "/DataSet/Persistor");
		//-- root-level persistor. this is used, among other things, to spawn engine by pid
		safespawn(engLog, newsname("ForecasterPersistor"), defaultdbg, engCfg, "/Engine/Persistor");
		//-- spawn engine from savedEnginePid_ with pid
		safespawn(engine, newsname("Engine"), defaultdbg, engCfg, "/Engine", shape->sampleLen*inferDS->selectedFeaturesCnt, shape->predictionLen*inferDS->selectedFeaturesCnt, engLog, savedEnginePid_);

		//=====================================================================================================================================================================
		//-- 5.1 create client persistor, if needed
		bool saveClient;
		safecall(clientCfg, setKey, "/Client");
		safecall(clientCfg->currentKey, getParm, &saveClient, "saveClient");
		safespawn(clientLog, newsname("ClientLogger"), defaultdbg, clientCfg, "Persistor");

		//-- 5.2 set simulation dates
		mallocSimulationDates(clientCfg, &simulationLength, &simulationTrainStartDate, &simulationInferStartDate, &simulationValidStartDate);
		safecall(this, mallocSimulationDates, clientCfg, &simulationLength, &simulationTrainStartDate, &simulationInferStartDate, &simulationValidStartDate);
		//=====================================================================================================================================================================

		//-- 6. get simulation date for s
		safecall(clientCfg->currentKey, getParm, simulationInferStartDate, "inferStartDate");
		getStartDates(inferDS, simulationInferStartDate[0], simulationLength, &simulationInferStartDate);
		//-- 6.0 for each simulation date,
		for (int s=0; s<simulationLength; s++) {

			//-- 6.1. start timer
			timer->start();
			//-- 6.2. set date0 in testDS->TimeSerie, and load it
			safecall(inferDS->sourceTS, load, TARGET, BASE, simulationInferStartDate[s]);
			//-- 6.3. do inference (also populates datasets)
			safecall(engine, infer, s, inferDS);
			//-- 6.4. persist Run logs
			safecall(engine, saveRun);
			//-- 6.5 Commit engine persistor data
			safecall(engine, commit);

			//-- 6.6. stop timer, and save client info
			endtimeS=timer->stop();
			safecall(clientLog, saveClientInfo, pid, s, "Root.Tester", timer->startTime, timer->elapsedTime, simulationTrainStartDate[s], simulationInferStartDate[s], simulationValidStartDate[s], false, true, false);
			//-- 5.4 Commit clientpersistor data
			safecall(clientLog, commit);

		}

	}
	catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}


}

void sRoot::mallocSimulationDates(sCfg* clientCfg_, int* simLen, char*** simTrainStart, char*** simInferStart, char*** simValidStart) {

	//-- get Simulation Length and start date[0]
	safecall(clientCfg_->currentKey, getParm, simLen, "SimulationLength");
	int sl=(*simLen);
	//--
	(*simTrainStart)=(char**)malloc(sl*sizeof(char*));
	(*simInferStart)=(char**)malloc((*simLen)*sizeof(char*));
	(*simValidStart)=(char**)malloc((*simLen)*sizeof(char*));
	for (int s=0; s<(*simLen); s++) {
		(*simTrainStart)[s]=(char*)malloc(DATE_FORMAT_LEN); (*simTrainStart)[s][0]='\0';
		(*simInferStart)[s]=(char*)malloc(DATE_FORMAT_LEN); (*simInferStart)[s][0]='\0';
		(*simValidStart)[s]=(char*)malloc(DATE_FORMAT_LEN); (*simValidStart)[s][0]='\0';
	}

}
void sRoot::getSimulationDates(sCfg* clientCfg_, int* simLen, char** simTrainStart, char** simInferStart, char** simValidStart) {

	//-- if the dataset is used, read startdate from client xml for each dataset
	if (forecaster->doTraining) {
		safecall(clientCfg_->currentKey, getParm, &simTrainStart[0], "TrainStartDate");
		getStartDates(forecaster->trainDS, simTrainStart[0], (*simLen), &simTrainStart);
	}
	if (forecaster->doInference) {
		safecall(clientCfg_->currentKey, getParm, &simInferStart[0], "InferStartDate");
		getStartDates(forecaster->inferDS, simInferStart[0], (*simLen), &simInferStart);
	}
	if (forecaster->doValidation) {
		safecall(clientCfg_->currentKey, getParm, &simValidStart[0], "ValidationStartDate");
		getStartDates(forecaster->validDS, simValidStart[0], (*simLen), &simValidStart);
	}

}

//-- utils stuff
void sRoot::CLoverride(int argc, char* argv[]) {
		char orName[XMLKEY_PARM_NAME_MAXLEN];
		char orValS[XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];

		//-- set default forecasterCfgFileFullName
		getFullPath("Tester.xml", clientCfgFileFullName);
		getFullPath("Forecaster.xml", forecasterCfgFileFullName);

		for (int p=1; p<argc; p++) {
			if (!getValuePair(argv[p], &orName[0], &orValS[0], '=')) fail("wrong parameter format in command line: %s", argv[p]);

			if (_stricmp(orName, "--cfgFileF")==0) {
				//-- special parameters, 1: alternative configuration file (forecaster)
				if (!getFullPath(orValS, forecasterCfgFileFullName)) fail("could not set cfgFileFullName from override parameter: %s", orValS);
			} else if (_stricmp(orName, "--cfgFileC")==0) {
				//-- special parameters, 1: alternative configuration file (forecaster)
				if (!getFullPath(orValS, clientCfgFileFullName)) fail("could not set cfgFileFullName from override parameter: %s", orValS);
			} else if (_stricmp(orName, "--VerboseRoot")==0) {
				dbg->verbose=(_stricmp(orValS, "TRUE")==0);
			} else {
				strcpy_s(cfgOverrideName[cfgOverrideCnt], XMLKEY_PARM_NAME_MAXLEN, orName);
				strcpy_s(cfgOverrideValS[cfgOverrideCnt], XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT, orValS);
				cfgOverrideCnt++;
			}
		}
	}
void sRoot::getStartDates(sDataSet* ds, char* date00_, int len, char*** oDates){
	sFXDataSource* fxsrc; sGenericDataSource* filesrc; sMT4DataSource* mt4src;
	switch (ds->sourceTS->sourceData->type) {
	case DB_SOURCE:
		fxsrc = (sFXDataSource*)ds->sourceTS->sourceData;
		safecall(fxsrc, getStartDates, date00_, len, oDates);
		break;
	case FILE_SOURCE:
		filesrc = (sGenericDataSource*)ds->sourceTS->sourceData;
		safecall(filesrc, getStartDates, date00_, len, oDates);
		break;
	case MT4_SOURCE:
		mt4src = (sMT4DataSource*)ds->sourceTS->sourceData;
		//--.............
		break;
	default:
		fail("Invalid ds->sourceTS->sourceData->type (%d)", ds->sourceTS->sourceData->type);
		break;
	}
}

//-- temp stuff
void sRoot::kaz() {


	//sCfg* ds1Cfg=new sCfg(this, newsname("ds1Cfg"), defaultdbg, "Config.New/Light/Infer.xml");
	//sDataSet* ds1=new sDataSet(this, newsname("ds1"), defaultdbg, ds1Cfg, "DataSet", 100, 3);

	sOraData* oradb1=new sOraData(this, newsname("oradb1"), defaultdbg, "History", "HistoryPwd", "Algo");
	sFXDataSource* fxsrc1=new sFXDataSource(this, newsname("FXDataSource1"), defaultdbg, oradb1, "EURUSD", "H1", false);
	sTimeSerie* ts1 = new sTimeSerie(this, newsname("TimeSerie1"), defaultdbg, fxsrc1, "201710010000", 202, DT_DELTA, "c:/temp/DataDump");

	const int selFcnt=4; int selF[selFcnt]={ 0,1,2,3 };
	int sampleLen=10; int predictionLen=3;
	sDataSet* ds1= new sDataSet(this, newsname("DataSet1"), defaultdbg, ts1, sampleLen, predictionLen, 10, selFcnt, selF, false, "C:/Temp/DataDump");

	ts1->load(TARGET, BASE, "201710010000");
	ts1->dump(TARGET, BASE);

	ts1->transform(TARGET, DT_DELTA);
	ts1->dump(TARGET, TR);

	ts1->scale(TARGET, TR, -1, 1);
	ts1->dump(TARGET, TRS);

	//-- Engine work... copy TARGET_TRS->PREDICTED_TRS, AND SKIP FIRST SAMPLE!
	int idx;
	for (int s=0; s<ts1->stepsCnt; s++) {
		for (int f=0; f<ts1->sourceData->featuresCnt; f++) {
			idx=s*ts1->sourceData->featuresCnt+f;
			ts1->val[PREDICTED][TRS][idx]=ts1->val[TARGET][TRS][idx];
		}
	}
	ts1->dump(TARGET, TRS);
	ts1->dump(PREDICTED, TRS);

	ts1->unscale(PREDICTED, -1, 1, selFcnt, selF, sampleLen );
	ts1->dump(PREDICTED, TR);

	ts1->untransform(PREDICTED, PREDICTED, sampleLen, selFcnt, selF);
	ts1->dump(PREDICTED, BASE);
}

void sRoot::trainAndRun(const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_) {

	//-- full filenames
	char clientffname[MAX_PATH];
	char shapeffname[MAX_PATH];
	char trainffname[MAX_PATH];
	char engineffname[MAX_PATH];

	//-- client vars
	int simulationLength;
	char** simulationTrainStartDate=nullptr;
	char** simulationInferStartDate=nullptr;
	char** simulationValidStartDate=nullptr;

	char* endtimeS;
	sTimer* timer=new sTimer();

	sDataShape* shape;
	sDataSet* trainDS; sLogger* trainLog;
	sEngine* engine;
	sLogger* clientLog;


	try {
		//-- 0. set full file name for each of the input files
		getFullPath(clientXMLfile_, clientffname);
		getFullPath(shapeXMLfile_, shapeffname);
		getFullPath(trainXMLfile_, trainffname);
		getFullPath(engineXMLfile_, engineffname);

		//-- 1. load separate sCfg* for client, dataShape, trainDataset, Engine
		sCfg* clientCfg; safespawn(clientCfg, newsname("clientCfg"), defaultdbg, clientffname);
		sCfg* shapeCfg; safespawn(shapeCfg, newsname("shapeCfg"), defaultdbg, shapeffname);
		sCfg* trainCfg; safespawn(trainCfg, newsname("trainCfg"), defaultdbg, trainffname);
		sCfg* engCfg; safespawn(engCfg, newsname("engineCfg"), defaultdbg, engineffname);

		//-- 2. spawn DataShape
		safespawn(shape, newsname("TrainDataShape"), defaultdbg, shapeCfg, "/DataShape");
		//-- 3. spawn Train DataSet and its persistor
		safespawn(trainDS, newsname("TrainDataSet"), defaultdbg, trainCfg, "/DataSet", shape->sampleLen, shape->predictionLen);
		safespawn(trainLog, newsname("TrainLogger"), defaultdbg, trainCfg, "/DataSet/Persistor");
		//-- 4. spawn engine the standard way
		safespawn(engine, newsname("TrainEngine"), defaultdbg, engCfg, "/Engine", shape->sampleLen*trainDS->selectedFeaturesCnt, shape->predictionLen*trainDS->selectedFeaturesCnt);

		//=====================================================================================================================================================================
		//-- 5.1 create client persistor, if needed
		bool saveClient;
		safecall(clientCfg, setKey, "/Client");
		safecall(clientCfg->currentKey, getParm, &saveClient, "saveClient");
		safespawn(clientLog, newsname("ClientLogger"), defaultdbg, clientCfg, "Persistor");

		//-- 5.2 set simulation dates
		safecall(this, mallocSimulationDates, clientCfg, &simulationLength, &simulationTrainStartDate, &simulationInferStartDate, &simulationValidStartDate);
		//=====================================================================================================================================================================

		//-- 5. for each simulation date,
		for (int s=0; s<simulationLength; s++) {
			//-- 5.1. start timer
			timer->start();
			//-- 5.2. get simulation date for s
			safecall(clientCfg->currentKey, getParm, simulationTrainStartDate, "TrainStartDate");
			getStartDates(trainDS, simulationTrainStartDate[0], simulationLength, &simulationTrainStartDate);
			//-- set date0 in trainDS->TimeSerie, and load it
			safecall(trainDS->sourceTS, load, TARGET, BASE, simulationTrainStartDate[s]);
			//-- do training (also populates datasets)
			safecall(engine, train, s, trainDS);
			//-- persist MSE logs
			safecall(engine, saveMSE);
			//-- persist Core logs
			safecall(engine, saveImage);
			//-- persist Engine Info
			safecall(engine, saveInfo);

			//============================================================================================================
			safecall(engine, infer, s, trainDS);
			//-- 6.4. persist Run logs
			safecall(engine, saveRun);
			//============================================================================================================

			//-- 5.4 Commit engine persistor data
			safecall(engine, commit);

			//-- 5.5. stop timer, and save client info
			endtimeS=timer->stop();
			safecall(clientLog, saveClientInfo, pid, s, "Root.Tester", timer->startTime, timer->elapsedTime, simulationTrainStartDate[s], simulationInferStartDate[s], simulationValidStartDate[s], true, false, false);
			//-- 5.4 Commit clientpersistor data
			safecall(clientLog, commit);

			//-- 
		}

	}
	catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}


}
