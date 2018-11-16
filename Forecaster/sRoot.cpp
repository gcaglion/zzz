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


	try{
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
		safespawn(trainDS, newsname("TrainDataSet"), defaultdbg, trainCfg, "/Train/DataSet", shape->sampleLen, shape->predictionLen);
		safespawn(trainLog, newsname("TrainLogger"), defaultdbg, trainCfg, "/Train/Persistor");
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

	} catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}


}
void sRoot::inferClient(const char* clientXMLfile_, const char* shapeXMLfile_, const char* inferXMLfile_, int savedEnginePid_) {

}


void sRoot::newClient() {

	char* endtimeS;
	sTimer* timer=new sTimer();
	sLogger* clientPersistor=nullptr;
	sCfg* clientCfg;
	sCfg* forecasterCfg;

	//-- client vars
	int simulationLength;
	char** simulationTrainStartDate=nullptr;
	char** simulationInferStartDate=nullptr;
	char** simulationValidStartDate=nullptr;


	try {

		//-- 1. load client XML configurations
		safespawn(clientCfg, newsname("clientCfg_Root"), defaultdbg, clientCfgFileFullName);

		//-- 2. create client persistor, if needed
		bool saveClient;
		safecall(clientCfg, setKey, "/Client");
		safecall(clientCfg->currentKey, getParm, &saveClient, "saveClient");
		if (saveClient) safespawn(clientPersistor, newsname("Client_Persistor"), defaultdbg, clientCfg, "Persistor");

		//-- 3. set simulation dates
		safecall(this, mallocSimulationDates, clientCfg, &simulationLength, &simulationTrainStartDate, &simulationInferStartDate, &simulationValidStartDate);

		//-- 4. Load Forecaster configuration
		safespawn(forecasterCfg, newsname("forecasterCfg_Root"), erronlydbg, forecasterCfgFileFullName);


		//-- 5. for each simulation date,
		for (int s=0; s<simulationLength; s++) {
			//-- 5.0. start timer
			timer->start();
			//-- 5.1. spawn forecaster
			safespawn(forecaster, newsname("mainForecaster"), defaultdbg, forecasterCfg, "/Forecaster");
			//-- 5.2. get simulation date for s
			safecall(this, getSimulationDates, clientCfg, &simulationLength, simulationTrainStartDate, simulationInferStartDate, simulationValidStartDate);
			//-- 5.3. spawn engine (either from xml or db based on configuration)
			safecall(forecaster, setEngine);
			//-- 5.4. training block
			safecall(forecaster, trainBlock, s, simulationTrainStartDate[s]);
			//-- 5.5. inference block
			safecall(forecaster, inferBlock, s, simulationInferStartDate[s]);
			//-- 5.6 Commit engine persistor data
			safecall(forecaster->engine, commit);
			//-- 5.7. stop timer, and save client info
			endtimeS=timer->stop();
			safecall(clientPersistor, saveClientInfo, pid, s, "Root.Tester", timer->startTime, timer->elapsedTime, simulationTrainStartDate[s], simulationInferStartDate[s], simulationValidStartDate[s], forecaster->doTraining, forecaster->doInference, forecaster->doTraining);

			//-- 5.8. delete forecaster (and engine along with it)
			delete forecaster;
		}

	} catch (std::exception exc) {
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


/*
void sRoot::tester() {

	//-- client vars
	int simulationLength;
	char** simulationTrainStartDate;
	char** simulationInferStartDate;
	char** simulationValidStartDate;

	sTimer* timer = new sTimer();

	try {

		//-- 0. take start time
		timer->start();

		//-- 1. load client XML configurations
		safespawn(clientCfg, newsname("clientCfg_Root"), defaultdbg, clientCfgFileFullName);

		//-- 2. create client persistor, if needed
		bool saveClient;
		safecall(clientCfg, setKey, "/Client");
		safecall(clientCfg->currentKey, getParm, &saveClient, "saveClient");
		if(saveClient) safespawn(clientPersistor, newsname("Client_Persistor"), defaultdbg, clientCfg, "Persistor");
		
		//-- 3.	get Simulation Length and start date[0] from client config
		safecall(clientCfg->currentKey, getParm, &simulationLength, "SimulationLength");


		//--
		simulationTrainStartDate=(char**)malloc(simulationLength*sizeof(char*));
		simulationInferStartDate=(char**)malloc(simulationLength*sizeof(char*));
		simulationValidStartDate=(char**)malloc(simulationLength*sizeof(char*));
		for (int s=0; s<simulationLength; s++) {
			simulationTrainStartDate[s]=(char*)malloc(DATE_FORMAT_LEN); simulationTrainStartDate[s][0]='\0';
			simulationInferStartDate[s]=(char*)malloc(DATE_FORMAT_LEN); simulationInferStartDate[s][0]='\0';
			simulationValidStartDate[s]=(char*)malloc(DATE_FORMAT_LEN); simulationValidStartDate[s][0]='\0';
		}

		//-- 4. Load Forecaster configuration
		safespawn(forecasterCfg, newsname("forecasterCfg_Root"), erronlydbg, forecasterCfgFileFullName);
		//-- 4.1. spawn forecaster
		safespawn(forecaster, newsname("mainForecaster"), defaultdbg, forecasterCfg, "/Forecaster");

		//-- 5. if the dataset is used, read startdate from client xml for each dataset
		if (forecaster->doTraining) {
			safecall(clientCfg->currentKey, getParm, &simulationTrainStartDate[0], "TrainStartDate");
			getStartDates(forecaster->trainDS, simulationTrainStartDate[0], simulationLength, &simulationTrainStartDate);
		}
		if (forecaster->doInference) {
			safecall(clientCfg->currentKey, getParm, &simulationInferStartDate[0], "InferStartDate");
			getStartDates(forecaster->inferDS, simulationInferStartDate[0], simulationLength, &simulationInferStartDate);
		}
		if (forecaster->doValidation) {
			safecall(clientCfg->currentKey, getParm, &simulationValidStartDate[0], "ValidationStartDate");
			getStartDates(forecaster->validDS, simulationValidStartDate[0], simulationLength, &simulationValidStartDate);
		}

		//-- 6. for each simulation
		for (int s=0; s<simulationLength; s++) {

			//-- 6.1. Training
			if (forecaster->doTraining) {
				//-- 6.1.1. set date0 in trainDS->TimeSerie, and load it
				safecall(forecaster->trainDS->sourceTS, load, TARGET, BASE, simulationTrainStartDate[s]);
				//-- 6.1.2. spawn engine from config file

				//-- 6.1.3. do training (also populates datasets)
				safecall(forecaster->engine, train, s, forecaster->trainDS);
				//-- 6.1.4. persist MSE logs
				safecall(forecaster->engine, saveMSE);
				//-- 6.1.5 persist Core logs
				safecall(forecaster->engine, saveImage);
				//-- 6.1.6 persist Engine Info
				safecall(forecaster->engine, saveInfo);
			}

			//-- 6.2. Inference
			if (forecaster->doInference) {
				//-- 6.2.1. set date0 in testDS->TimeSerie, and load it
				safecall(forecaster->inferDS->sourceTS, load, TARGET, BASE, simulationInferStartDate[s]);

				//-- 6.2.2. do inference (also populates datasets)
				safecall(forecaster->engine, infer, s, forecaster->inferDS);
				//-- 6.2.3. persist Run logs
				safecall(forecaster->engine, saveRun);
			}

			//-- 6.3. Save ClientInfo
			if (saveClient) {
				char* endtimeS = timer->stop();
				safecall(clientPersistor, saveClientInfo, pid, s, "Root.Tester", timer->startTime, timer->elapsedTime, simulationTrainStartDate[s], simulationInferStartDate[s], simulationValidStartDate[s], forecaster->doTraining, forecaster->doInference, forecaster->doTraining);
			}

			//-- 6.4 Commit engine persistor data
			safecall(forecaster->engine, commit);

			//-- 6.5. commit client persistor data
			safecall(clientPersistor, commit);

		}

	} catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}
}
*/
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

//-- MetaTrader calls
bool MTcreateForecasterEnv(char* baseConfigFileFullName, int overridesCnt, char* overridesFullString, char* oRootObjPointerString) {
	return false;
}

//-- temp stuff
/*
numtype sRoot::MyRndDbl(numtype min, numtype max) {
	unsigned int number;
	int err;
	numtype ret;

	err = rand_s(&number);
	ret = min+(numtype)number/((numtype)UINT_MAX+1) * (max-min);
	return ret;
}
void sRoot::testDML() {

	sOraData* oradb1;
	safespawn(oradb1, newsname("InferOraDB"), defaultdbg, "CULogUser", "LogPwd", "Algo");
	sOraData* oradb2;
	safespawn(oradb2, newsname("InferOraHistory"), defaultdbg, "History", "HistoryPwd", "Algo");

	int sdatecnt=10;
	char** sdate=(char**)malloc(sdatecnt*sizeof(char*)); for (int i=0; i<sdatecnt; i++) sdate[i]=(char*)malloc(DATE_FORMAT_LEN);
	oradb1->getStartDates("EURUSD", "H1", false, "201612300000", sdatecnt, &sdate);


	int pid=99;
	int epochs=2000;
	int tid=pid;
	numtype* trainMSE = (numtype*)malloc(epochs*sizeof(numtype));
	numtype* validMSE = (numtype*)malloc(epochs*sizeof(numtype));
	for (int e=0; e<epochs; e++) {
		trainMSE[e]=MyRndDbl(0, 10);
		validMSE[e]=MyRndDbl(0, 10);
	}
	safecall(oradb1, saveMSE, pid, tid, epochs, trainMSE, validMSE);

	int setid=0, npid=pid, ntid=tid, barsCnt=1000, featuresCnt=4;
	int feature[4]={ 0,1,2,3 };
	int predictionLen=3;
	numtype* prediction= (numtype*)malloc(barsCnt*predictionLen*featuresCnt*sizeof(numtype));
	numtype* actual    = (numtype*)malloc(barsCnt*predictionLen*featuresCnt*sizeof(numtype));

	int i=0;
	for (int b=0; b<barsCnt; b++) {
		for (int f=0; f<featuresCnt; f++) {
			prediction[i]=MyRndDbl(0, 10);
			actual[i]=MyRndDbl(0, 10);
			i++;
		}
	}
	//safecall(oradb1, saveRun, pid, tid, npid, ntid, barsCnt, featuresCnt, feature, predictionLen, predictionTRS, actual);

	int Wcnt=35150;
	numtype* W = (numtype*)malloc(Wcnt*sizeof(numtype));
	//for (int i=0; i<Wcnt; i++) W[i]=MyRndDbl(0, 1);
	//safecall(oradb1, SaveW, pid, tid, 2000, Wcnt, W);
	pid=73624;
	tid=76992;
	int epoch=-1;
	//safecall(oradb1, loadW, pid, tid, epoch, Wcnt, W);

}
void sRoot::kaz4() {

	printf("ProcessId: %d\n\n", GetCurrentProcessId());

	sOraData* oradb1; safespawn(oradb1, newsname("oradb1"), defaultdbg, "History", "HistoryPwd", "Algo.mavi");
	sFXDataSource* fxdatasrc1; safespawn(fxdatasrc1, newsname("datasrc1"), defaultdbg, oradb1, "EURUSD", "H1", false);
	const int TSFcnt=1; int TSF[TSFcnt]={ 2 };
	sTimeSerie* ts1; safespawn(ts1, newsname("ts1"), defaultdbg, fxdatasrc1, "201608010000", 202, DT_DELTA, TSFcnt, TSF, "C:/temp/DataDump");

	ts1->load(TARGET, BASE);
	ts1->dump(TARGET, BASE);
	ts1->transform(TARGET);
	ts1->dump(TARGET, TR);
	ts1->scale(TARGET, TR, -1, 1);
	ts1->dump(TARGET, TRS);

	ts1->dump(PREDICTED, TRS);
	ts1->unscale(PREDICTED, -1, 1, TSFcnt, TSF, 0);
	ts1->dump(PREDICTED, TR);
	ts1->untransform(PREDICTED, TSFcnt, TSF);
	ts1->dump(PREDICTED, BASE);
	
	const int selFcnt=2; int selF[selFcnt]={ 1,2 };
	sDataSet* ds1 = new sDataSet(this, newsname("ds1"), defaultdbg, ts1, 10, 3, 10, selFcnt, selF, true, "C:/temp/DataDump");
	ds1->build(TARGET, BASE);

	ds1->reorder(SAMPLE, SBF, BFS);
	ds1->reorder(TARGET, SBF, BFS);

	//-- core work...
	size_t bfsSize=ds1->samplesCnt*ds1->predictionLen*ds1->selectedFeaturesCnt;
	//-- perfect core!
	for (int i=0; i<bfsSize; i++) ds1->predictionBFS[i]=ds1->targetBFS[i];

	ds1->reorder(PREDICTED, BFS, SBF);

	ds1->unbuild(PREDICTED, PREDICTED, BASE);	//-- this means, unbuild from PREDICTED section of DataSet into PREDICTED-BASE section of TimeSerie
	ts1->dump(PREDICTED, BASE);

	return;

	sDataSet* ds2 = new sDataSet(this, newsname("ds2_using_ds1"), defaultdbg, ds1);
	ds2->build(TARGET, BASE);
	ds2->unbuild(TARGET, PREDICTED, BASE);
	ts1->dump(PREDICTED, BASE);
	return;
//	sDataSet* ds2 = new sDataSet(this, newsname("ds1"), defaultdbg, ts1, 10, 3, 10, selFcnt, selF, false, nullptr, true, "C:/temp/DataDump");
//	ds2->build(TARGET, TR);
//	sDataSet* ds3 = new sDataSet(this, newsname("ds1"), defaultdbg, ts1, 10, 3, 10, selFcnt, selF, false, nullptr, true, "C:/temp/DataDump");
//	ds3->build(TARGET, TRS);
	


}
void sRoot::kaz5() {

	int pid=GetCurrentProcessId();
	int tid=GetCurrentThreadId();
	printf("ProcessId: %d ; ThreadId: %d\n\n", pid, tid);

	sOraData* oradb1; safespawn(oradb1, newsname("oradb1"), defaultdbg, "LogUser", "LogPwd", "Algo");
	sLogger* pers1; safespawn(pers1, newsname("pers1"), defaultdbg, oradb1);
	safecall(pers1, open);

	safespawn(forecasterCfg, newsname("forecasterCfg_Root"), erronlydbg, forecasterCfgFileFullName);

	sNNparms* nn1parms; safespawn(nn1parms, newsname("NN1Parms"), defaultdbg, forecasterCfg, "/Forecaster/Engine/Custom/Core0/Parameters");
	nn1parms->save(pers1, pid, tid);
	pers1->commit();

	sNNparms* nn2parms; safespawn(nn2parms, newsname("NN2Parms"), defaultdbg, pers1, pid, tid);

}
*/