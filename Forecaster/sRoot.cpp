#include "sRoot.h"

sRoot::sRoot(int argc_, char* argv_[]) : sObj(nullptr, newsname("RootObj"), nullptr) {
		dbg->pauseOnError=true;

		CLoverride(argc_, argv_);
	}
sRoot::~sRoot() {}

void sRoot::tester() {

	int simulationLength;
	char** simulationTrainStartDate;
	char** simulationValidStartDate;
	char** simulationTestStartDate;
	sClientLogger* testerPersistor;

	try {

		//-- 1. load tester and Forecaster XML configurations
		safespawn(testerCfg, newsname("testerCfg_Root"), dbg, testerCfgFileFullName);
		safespawn(forecasterCfg, newsname("forecasterCfg_Root"), dbg, forecasterCfgFileFullName);

		//-- 2. save tester Log (elapsedTime is 0)

		//-- 3. create tester persistor
		safespawn(testerPersistor, newsname("Client_Persistor"), defaultdbg, testerCfg, "/Client/Persistor");
		
		//-- 4.	get Simulation Length
		safecall(testerCfg->currentKey, getParm, &simulationLength, "Client/SimulationLength");
		
		//-- 5. malloc simulation start dates for each Dataset
		simulationTrainStartDate=(char**)malloc(simulationLength*sizeof(char*));
		simulationValidStartDate=(char**)malloc(simulationLength*sizeof(char*));
		simulationTestStartDate =(char**)malloc(simulationLength*sizeof(char*));
		for (int d=0; d<simulationLength; d++) {
			simulationTrainStartDate[d]=(char*)malloc(DATE_FORMAT_LEN);
			simulationValidStartDate[d]=(char*)malloc(DATE_FORMAT_LEN);
			simulationTestStartDate[d] =(char*)malloc(DATE_FORMAT_LEN);
		}

		//-- 6. spawn forecaster
		safespawn(forecaster, newsname("mainForecaster"), defaultdbg, forecasterCfg, "/Forecaster");

		//-- 7. for each used dataset,
		if (forecaster->data->doTraining) {
			//-- 7.1. get simulation start dates
			safecall(this, getStartDates, forecaster->data->trainDS, simulationLength, simulationTrainStartDate);
			//-- 7.2. get source data into datasets
		}
		if (forecaster->data->doInference) {
			//-- 7.1. get simulation start dates
			safecall(this, getStartDates, forecaster->data->testDS, simulationLength, simulationTestStartDate);
		}
		if (forecaster->data->doValidation) {
			//-- 7.1. get simulation start dates
			safecall(this, getStartDates, forecaster->data->validDS, simulationLength, simulationValidStartDate);
		}



		//-- 4.3. do actions (train/test/validate) according to configuration
		if (forecaster->data->doTraining) safecall(forecaster->engine, train, forecaster->data->trainDS);		
		if (forecaster->data->doInference) safecall(forecaster->engine, infer, forecaster->data->testDS);

		//-- 4.4 save logs (completely rivisited in Logger_Rehaul branch)
		forecaster->engine->saveMSE();
		//-- 4.5. delete forecaster


	}
	catch (std::exception exc) {
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
		getFullFileName("../Tester.xml", testerCfgFileFullName);
		getFullFileName("../Forecaster.xml", forecasterCfgFileFullName);

		for (int p=1; p<argc; p++) {
			if (!getValuePair(argv[p], &orName[0], &orValS[0], '=')) fail("wrong parameter format in command line: %s", argv[p]);
			if (_stricmp(orName, "--cfgFile")==0) {
				if (!getFullFileName(orValS, forecasterCfgFileFullName)) fail("could not set cfgFileFullName from override parameter: %s", orValS);
			} else {
				strcpy_s(cfgOverrideName[cfgOverrideCnt], XMLKEY_PARM_NAME_MAXLEN, orName);
				strcpy_s(cfgOverrideValS[cfgOverrideCnt], XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT, orValS);
				cfgOverrideCnt++;
			}
		}
	}
void sRoot::testDML() {

	sOraData* oradb1;
	safespawn(oradb1, newsname("TestOraDB"), defaultdbg, "CULogUser", "LogPwd", "Algo", true);
	sOraData* oradb2;
	safespawn(oradb2, newsname("TestOraHistory"), defaultdbg, "History", "HistoryPwd", "Algo", true);

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
	numtype* prediction= (numtype*)malloc(barsCnt*featuresCnt*sizeof(numtype));
	numtype* actual    = (numtype*)malloc(barsCnt*featuresCnt*sizeof(numtype));

	int i=0;
	for (int b=0; b<barsCnt; b++) {
		for (int f=0; f<featuresCnt; f++) {
			prediction[i]=MyRndDbl(0, 10);
			actual[i]=MyRndDbl(0, 10);
			i++;
		}
	}
	safecall(oradb1, saveRun, pid, tid, setid, npid, ntid, barsCnt, featuresCnt, feature, prediction, actual);

	int Wcnt=35150;
	numtype* W = (numtype*)malloc(Wcnt*sizeof(numtype));
	//for (int i=0; i<Wcnt; i++) W[i]=MyRndDbl(0, 1);
	//safecall(oradb1, SaveW, pid, tid, 2000, Wcnt, W);
	pid=73624;
	tid=76992;
	int epoch=-1;
	safecall(oradb1, loadW, pid, tid, epoch, Wcnt, W);

}
void sRoot::getStartDates(sDataSet* ds, int len, char** oDates){
	sFXDataSource* fxsrc; sGenericDataSource* filesrc; sMT4DataSource* mt4src;
	switch (ds->sourceTS->sourceData->type) {
	case DB_SOURCE:
		fxsrc = (sFXDataSource*)ds->sourceTS->sourceData;
		safecall(fxsrc, getStartDates, ds->sourceTS->date0, len, oDates);
		break;
	case FILE_SOURCE:
		filesrc = (sGenericDataSource*)ds->sourceTS->sourceData;
		safecall(filesrc, getStartDates, ds->sourceTS->date0, len, oDates);
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