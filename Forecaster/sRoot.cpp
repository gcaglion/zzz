#include "sRoot.h"

//-- constructor / destructor
sRoot::sRoot(int argc_, char* argv_[]) : sObj(nullptr, newsname("RootObj"), defaultdbg) {
	dbg->verbose=false;

	pid=GetCurrentProcessId();
	CLoverride(argc_, argv_);
}
sRoot::~sRoot() {}

//-- core stuff
void sRoot::tester() {

	//-- client vars
	int simulationLength;
	char** simulationTrainStartDate;
	char** simulationInferStartDate;
	char** simulationValidStartDate;

	sTimer* timer = new sTimer();

	try {

		//-- 0. take client start time
		timer->start();

		//-- 1. load tester and Forecaster XML configurations
		safespawn(testerCfg, newsname("testerCfg_Root"), erronlydbg, testerCfgFileFullName);
		safespawn(forecasterCfg, newsname("forecasterCfg_Root"), erronlydbg, forecasterCfgFileFullName);

		//-- 2. create tester persistor
		safespawn(testerPersistor, newsname("Client_Persistor"), defaultdbg, testerCfg, "/Client/Persistor");
		
		//-- 3.	get Simulation Length and start date[0]
		safecall(testerCfg->currentKey, getParm, &simulationLength, "Client/SimulationLength");

		//--
		simulationTrainStartDate=(char**)malloc(simulationLength*sizeof(char*));
		simulationInferStartDate=(char**)malloc(simulationLength*sizeof(char*));
		simulationValidStartDate=(char**)malloc(simulationLength*sizeof(char*));
		for (int s=0; s<simulationLength; s++) {
			simulationTrainStartDate[s]=(char*)malloc(DATE_FORMAT_LEN); simulationTrainStartDate[s][0]='\0';
			simulationInferStartDate[s]=(char*)malloc(DATE_FORMAT_LEN); simulationInferStartDate[s][0]='\0';
			simulationValidStartDate[s]=(char*)malloc(DATE_FORMAT_LEN); simulationValidStartDate[s][0]='\0';
		}

		//-- 4. spawn forecaster
		safespawn(forecaster, newsname("mainForecaster"), defaultdbg, forecasterCfg, "/Forecaster");

		//-- 5. if the dataset is used, read startdate from client xml for each dataset
		if (forecaster->data->doTraining) {
			safecall(testerCfg->currentKey, getParm, &simulationTrainStartDate[0], "Client/TrainStartDate");
			getStartDates(forecaster->data->trainDS, simulationTrainStartDate[0], simulationLength, &simulationTrainStartDate);
		}
		if (forecaster->data->doInference) {
			safecall(testerCfg->currentKey, getParm, &simulationInferStartDate[0], "Client/InferStartDate");
			getStartDates(forecaster->data->testDS, simulationInferStartDate[0], simulationLength, &simulationInferStartDate);
		}
		if (forecaster->data->doValidation) {
			safecall(testerCfg->currentKey, getParm, &simulationValidStartDate[0], "Client/ValidationStartDate");
			getStartDates(forecaster->data->validDS, simulationValidStartDate[0], simulationLength, &simulationValidStartDate);
		}

		//-- 6. for each simulation
		for (int s=0; s<simulationLength; s++) {

			//-- 6.1. Training
			if (forecaster->data->doTraining) {
				//-- 6.1.1. set date0 in trainDS->TimeSerie, and load it
				safecall(forecaster->data->trainDS->sourceTS, load, TARGET, BASE, simulationTrainStartDate[s]);
				//-- 6.1.2. do training (also populates datasets)
				safecall(forecaster->engine, train, s, forecaster->data->trainDS);
				//-- 6.1.3. persist MSE logs
				safecall(forecaster->engine, saveMSE);
				//-- 6.1.4 destroy trining-related objects
			}

			//-- 6.2. Inference
			if (forecaster->data->doInference) {
				//-- 6.2.1. set date0 in testDS->TimeSerie, and load it
				safecall(forecaster->data->testDS->sourceTS, load, TARGET, BASE, simulationInferStartDate[s]);
				//-- 6.2.2. do training (also populates datasets)
				safecall(forecaster->engine, infer, s, forecaster->data->testDS);
				//-- 6.2.3. persist Run logs
				safecall(forecaster->engine, saveRun);
			}

			//-- 6.3. Save ClientInfo
			if (testerPersistor->saveClientInfoFlag) {
				char* endtimeS = timer->stop();
				safecall(testerPersistor, saveClientInfo, pid, s, "Root.Tester", timer->startTime, timer->elapsedTime, simulationTrainStartDate[s], simulationInferStartDate[s], simulationValidStartDate[s], forecaster->data->doTraining, forecaster->data->doInference, forecaster->data->doTraining);
			}

			//-- 6.4 Commit engine persistor data
			safecall(forecaster->engine, commit);

			//-- 6.5. commit client persistor data
			safecall(testerPersistor, commit);

		}

	} catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}
}

//-- utils stuff
void sRoot::CLoverride(int argc, char* argv[]) {
		char orName[XMLKEY_PARM_NAME_MAXLEN];
		char orValS[XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];

		//-- set default forecasterCfgFileFullName
		getFullPath("../Tester.xml", testerCfgFileFullName);
		getFullPath("../Forecaster.xml", forecasterCfgFileFullName);

		for (int p=1; p<argc; p++) {
			if (!getValuePair(argv[p], &orName[0], &orValS[0], '=')) fail("wrong parameter format in command line: %s", argv[p]);

			if (_stricmp(orName, "--cfgFile")==0) {
				//-- special parameters, 1: alternative configuration file
				if (!getFullPath(orValS, forecasterCfgFileFullName)) fail("could not set cfgFileFullName from override parameter: %s", orValS);
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
	safecall(oradb1, loadW, pid, tid, epoch, Wcnt, W);

}
void sRoot::kaz4() {

	printf("ProcessId: %d\n\n", GetCurrentProcessId());

	sOraData* oradb1; safespawn(oradb1, newsname("oradb1"), defaultdbg, "History", "HistoryPwd", "Algo.mavi");
	sFXDataSource* fxdatasrc1; safespawn(fxdatasrc1, newsname("datasrc1"), defaultdbg, oradb1, "EURUSD", "H1", false);
	const int TSFcnt=1; int TSF[TSFcnt]={ 2 };
	sTimeSerie* ts1; safespawn(ts1, newsname("ts1"), defaultdbg, fxdatasrc1, "201608010000", 202, DT_DELTA, TSFcnt, TSF, "C:/temp/DataDump");

	ts1->load(TARGET, BASE);
	ts1->dump(TARGET, BASE);
	/*	ts1->transform(TARGET);
	ts1->dump(TARGET, TR);
	ts1->scale(TARGET, TR, -1, 1);
	ts1->dump(TARGET, TRS);

	ts1->dump(PREDICTED, TRS);
	ts1->unscale(PREDICTED, -1, 1, TSFcnt, TSF, 0);
	ts1->dump(PREDICTED, TR);
	ts1->untransform(PREDICTED, TSFcnt, TSF);
	ts1->dump(PREDICTED, BASE);
	*/
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
	/*		sDataSet* ds2 = new sDataSet(this, newsname("ds1"), defaultdbg, ts1, 10, 3, 10, selFcnt, selF, false, nullptr, true, "C:/temp/DataDump");
	ds2->build(TARGET, TR);
	sDataSet* ds3 = new sDataSet(this, newsname("ds1"), defaultdbg, ts1, 10, 3, 10, selFcnt, selF, false, nullptr, true, "C:/temp/DataDump");
	ds3->build(TARGET, TRS);
	*/


}
