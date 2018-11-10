#include "sRoot.h"

sRoot::sRoot(int argc_, char* argv_[]) : sObj(nullptr, newsname("RootObj"), defaultdbg) {
	dbg->verbose=false;

	pid=GetCurrentProcessId();
	CLoverride(argc_, argv_);
}
sRoot::~sRoot() {}

void sRoot::kaz4() {
	
	sOraData* oradb1; safespawn(oradb1, newsname("oradb1"), defaultdbg, "History", "HistoryPwd", "Algo");
	sFXDataSource* fxdatasrc1; safespawn(fxdatasrc1, newsname("datasrc1"), defaultdbg, oradb1, "EURUSD", "H1", false);
	const int TSFcnt=2; int TSF[TSFcnt]={ 1,2 };
	sTimeSerie* ts1; safespawn(ts1, newsname("ts1"), defaultdbg, fxdatasrc1, "201608010000", 202, DT_DELTA, TSFcnt, TSF);

	ts1->load();

//	ts1->scale(-1, 1);
//	ts1->unscale(-1, 1, TSFcnt, TSF, 100, ts1->trsvalA, ts1->trvalA);
//	ts1->untransform(TSFcnt, TSF, ts1->trvalA, ts1->valA);

	const int selFcnt=2; int selF[selFcnt]={ 1,2 };
	sDataSet* ds1 = new sDataSet(this, newsname("ds1"), defaultdbg, ts1, 10, 3, 10, selFcnt, selF, false);
//	ds1->build(-1, 1);
//	ds1->dump();

	sDataSet* ds2 = new sDataSet(this, newsname("ds2"), defaultdbg, ds1);
//	ds2->build(-1, 1, TRSVAL);
//	ds2->dump(TRSVAL, false);

	

}


void sRoot::tester() {

	//-- client vars
	int simulationLength;
	char** simulationTrainStartDate;
	char** simulationInferStartDate;
	char** simulationValidStartDate;
	sClientLogger* testerPersistor;
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
		simulationTrainStartDate=(char**)malloc(simulationLength*sizeof(char*)); for (int s=0; s<simulationLength; s++) simulationTrainStartDate[s]=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);
		simulationInferStartDate=(char**)malloc(simulationLength*sizeof(char*)); for (int s=0; s<simulationLength; s++) simulationInferStartDate[s]=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);
		simulationValidStartDate=(char**)malloc(simulationLength*sizeof(char*)); for (int s=0; s<simulationLength; s++) simulationValidStartDate[s]=(char*)malloc(XMLKEY_PARM_VAL_MAXLEN);

		//-- 4. spawn forecaster
		safespawn(forecaster, newsname("mainForecaster"), defaultdbg, forecasterCfg, "/Forecaster");

		//-- 5. if the dataset is used, read startdate from client xml for each dataset
		if (forecaster->data->doTraining) {
			safecall(testerCfg->currentKey, getParm, &simulationTrainStartDate[0], "Client/TrainStartDate");
			getStartDates(forecaster->data->trainDS, simulationTrainStartDate[0], simulationLength, simulationTrainStartDate);
		}
		if (forecaster->data->doInference) {
			safecall(testerCfg->currentKey, getParm, &simulationInferStartDate[0], "Client/InferStartDate");
			getStartDates(forecaster->data->testDS, simulationInferStartDate[0], simulationLength, simulationInferStartDate);
		}
		if (forecaster->data->doValidation) {
			safecall(testerCfg->currentKey, getParm, &simulationValidStartDate[0], "Client/ValidationStartDate");
			getStartDates(forecaster->data->validDS, simulationValidStartDate[0], simulationLength, simulationValidStartDate);
		}

		//-- 6. for each simulation
		for (int s=0; s<simulationLength; s++) {

			//-- 6.1. Training
			if (forecaster->data->doTraining) {
				//-- 6.1.1. set date0 in trainDS->TimeSerie, and load it
				safecall(forecaster->data->trainDS->sourceTS, load, simulationTrainStartDate[s]);
				//-- 6.1.2. do training (also populates datasets)
				safecall(forecaster->engine, train, s, forecaster->data->trainDS);
				//-- 6.1.3. persist MSE logs
				safecall(forecaster->engine, saveMSE);
				//-- 6.1.4 destroy trining-related objects
			}

			//-- 6.2. Inference
			if (forecaster->data->doInference) {
				//-- 6.2.1. set date0 in testDS->TimeSerie, and load it
				forecaster->data->testDS->sourceTS->load(simulationInferStartDate[s]);
				//-- 6.2.2. do training (also populates datasets)
				safecall(forecaster->engine, infer, s, forecaster->data->testDS);
				//-- 6.2.3. persist Run logs
				safecall(forecaster->engine, saveRun);
			}

			//-- 6.3 Commit engine persistor data
			safecall(forecaster->engine, commit);

			//-- 6.4. Save ClientInfo for
			if (testerPersistor->saveClientInfoFlag) {
				char* endtimeS = timer->stop();
				testerPersistor->saveClientInfo( pid, "Root.Tester", timer->startTime, timer->elapsedTime/1000, simulationLength, simulationTrainStartDate[s], forecaster->data->doTraining, forecaster->data->doInference, false);

			}

			//-- 6.5. commit client persistor data

		}

	} catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}
}
numtype sRoot::MyRndDbl(numtype min, numtype max) {
		unsigned int number;
		int err;
		numtype ret;

		err = rand_s(&number);
		ret = min+(numtype)number/((numtype)UINT_MAX+1) * (max-min);
		return ret;
	}
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
void sRoot::testDML() {

	sOraData* oradb1;
	safespawn(oradb1, newsname("InferOraDB"), defaultdbg, "CULogUser", "LogPwd", "Algo");
	sOraData* oradb2;
	safespawn(oradb2, newsname("InferOraHistory"), defaultdbg, "History", "HistoryPwd", "Algo");

	int sdatecnt=10;
	char** sdate=(char**)malloc(sdatecnt*sizeof(char*)); for (int i=0; i<sdatecnt; i++) sdate[i]=(char*)malloc(DATE_FORMAT_LEN);
	oradb1->getStartDates("EURUSD", "H1", false, "201612300000", sdatecnt, sdate);


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
void sRoot::getStartDates(sDataSet* ds, char* date00_, int len, char** oDates){
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