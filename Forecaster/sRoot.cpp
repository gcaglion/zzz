#include "sRoot.h"

sRoot::sRoot(int argc_, char* argv_[]) : sObj(nullptr, newsname("RootObj"), nullptr) {
		dbg->pauseOnError=true;

		CLoverride(argc_, argv_);
	}
sRoot::~sRoot() {}

void sRoot::tester() {

	int simulationLength;
	char** simulationTrainStartDate;
	char** simulationTestStartDate;
	char** simulationValidStartDate;
	sClientLogger* testerPersistor;

	try {

		//-- 1. load tester and Forecaster XML configurations
		safespawn(testerCfg, newsname("testerCfg_Root"), dbg, testerCfgFileFullName);
		safespawn(forecasterCfg, newsname("forecasterCfg_Root"), dbg, forecasterCfgFileFullName);

		//-- 2. create tester persistor
		safespawn(testerPersistor, newsname("Client_Persistor"), defaultdbg, testerCfg, "/Client/Persistor");
		
		//-- 3.	get Simulation Length and start date[0]
		safecall(testerCfg->currentKey, getParm, &simulationLength, "Client/SimulationLength");
		//--
		simulationTrainStartDate=(char**)malloc(simulationLength*sizeof(char*)); for (int s=0; s<simulationLength; s++) simulationTrainStartDate[s]=(char*)malloc(DATE_FORMAT_LEN);
		simulationTestStartDate=(char**)malloc(simulationLength*sizeof(char*)); for (int s=0; s<simulationLength; s++) simulationTestStartDate[s]=(char*)malloc(DATE_FORMAT_LEN);
		simulationValidStartDate=(char**)malloc(simulationLength*sizeof(char*)); for (int s=0; s<simulationLength; s++) simulationValidStartDate[s]=(char*)malloc(DATE_FORMAT_LEN);

		//-- 4. spawn forecaster
		safespawn(forecaster, newsname("mainForecaster"), defaultdbg, forecasterCfg, "/Forecaster");

		//-- 5. if the dataset is used, read startdate from client xml for each dataset
		if (forecaster->data->doTraining) {
			safecall(testerCfg->currentKey, getParm, &simulationTrainStartDate[0], "Client/TrainStartDate");
			getStartDates(forecaster->data->trainDS, simulationTrainStartDate[0], simulationLength, simulationTrainStartDate);
		}
		if (forecaster->data->doInference) {
			safecall(testerCfg->currentKey, getParm, &simulationTestStartDate[0], "Client/TestStartDate");
			getStartDates(forecaster->data->testDS, simulationTestStartDate[0], simulationLength, simulationTestStartDate);
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
				forecaster->engine->saveMSE();

			}
			//-- 6.2. Inference
			if (forecaster->data->doInference) {
				//-- 6.2.1. set date0 in testDS->TimeSerie, and load it
				forecaster->data->testDS->sourceTS->load(simulationTestStartDate[s]);
				//-- 6.2.2. do training (also populates datasets)
				safecall(forecaster->engine, infer, s, forecaster->data->testDS);
			}

		}

		//-- 4.4 save logs (completely rivisited in Logger_Rehaul branch)

		//-- 4.5. delete forecaster

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
				if (!getFullPath(orValS, forecasterCfgFileFullName)) fail("could not set cfgFileFullName from override parameter: %s", orValS);
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