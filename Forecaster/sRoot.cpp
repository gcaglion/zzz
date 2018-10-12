#include "sRoot.h"

sRoot::sRoot(int argc_, char* argv_[]) : sObj(nullptr, newsname("RootObj"), nullptr) {
		dbg->pauseOnError=true;

		CLoverride(argc_, argv_);
	}
sRoot::~sRoot() {}

void sRoot::execute(int what) {

	int d;
	int simulationLength;
	char** simulationTrainStartDate;
	char** simulationValidStartDate;
	char** simulationTestStartDate;

	//-- get fxdata info

	//--

	try {

		//-- 1. load client and Forecaster XML configurations
		safespawn(false, clientCfg, newsname("clientCfg_Root"), dbg, clientCfgFileFullName);
		safespawn(false, forecasterCfg, newsname("forecasterCfg_Root"), dbg, forecasterCfgFileFullName);

		//-- 2. save client Log (elapsedTime is 0)

		//-- 3. create client persistor
		safespawn(false, clientPersistor, newsname("Client_Persistor"), defaultdbg, clientCfg, "/Client/Persistor");
		
		//-- 4.	get Simulation Length
		safecall(clientCfg->currentKey, getParm, &simulationLength, "Client/SimulationLength");
		
		//-- 5. malloc simulation start dates for each Dataset
		simulationTrainStartDate=(char**)malloc(simulationLength*sizeof(char*)); for (d=0; d<simulationLength; d++)	simulationTrainStartDate[d]=(char*)malloc(12+1);
		simulationValidStartDate=(char**)malloc(simulationLength*sizeof(char*)); for (d=0; d<simulationLength; d++)	simulationValidStartDate[d]=(char*)malloc(12+1);
		simulationTestStartDate =(char**)malloc(simulationLength*sizeof(char*)); for (d=0; d<simulationLength; d++)	simulationTestStartDate[d] =(char*)malloc(12+1);

		//-- 6. spawn forecaster
		safespawn(false, forecaster, newsname("mainForecaster"), defaultdbg, forecasterCfg, "/Forecaster");

		//-- 7. get simulation start dates
		getStartDates(forecaster->data->trainDS, simulationLength, simulationTrainStartDate);
		getStartDates(forecaster->data->testDS, simulationLength, simulationTestStartDate);
		getStartDates(forecaster->data->validDS, simulationLength, simulationValidStartDate);

		//-- 4.3. do actions (train/test/validate) according to configuration
		if (forecaster->data->doTraining) {
			safecall(forecaster->engine, train, forecaster->data->trainDS);
		}
		if (forecaster->data->doInference) {
			safecall(forecaster->engine, infer, forecaster->data->testDS);
		}

		//-- 4.4. delete forecaster


	/*	//-- 4. for each simulation
		for (int d=0; d<simulationLength; d++) {

			//-- 4.1. save client Log (elapsedTime is 0)
			//-- 4.2. spawn forecaster
			safespawn(false, forecaster, newsname("Forecaster_%d_%s", d, simulationStartDate[d]), defaultdbg, forecasterCfg, "/Forecaster");

			//-- 4.3. do actions (train/test/validate) according to configuration
			//-- 4.4. delete forecaster
		}

		//-- 6. free simulationStartDate[]
		for (d=0; d<simulationLength; d++) {
			free(simulationTrainStartDate[d]);
			free(simulationValidStartDate[d]);
			free(simulationTestStartDate[d]);
		}
*/
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
		getFullFileName("../Client.xml", clientCfgFileFullName);
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
		sOraDB* oradb1;

		int pid=99;
		int epochs=2000;
		int tid=pid;
		numtype* trainMSE = (numtype*)malloc(epochs*sizeof(numtype));
		numtype* validMSE = (numtype*)malloc(epochs*sizeof(numtype));
		for (int e=0; e<epochs; e++) {
			trainMSE[e]=MyRndDbl(0, 10);
			validMSE[e]=MyRndDbl(0, 10);
		}
		safespawn(false, oradb1, newsname("TestOraDB"), defaultdbg, "CULogUser", "LogPwd", "Algo", true);
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
	sFXData* fxsrc; sFileData* filesrc; sMT4Data* mt4src;
	switch (ds->sourceTS->sourceData->type) {
	case FXDB_SOURCE:
		fxsrc = (sFXData*)ds->sourceTS->sourceData;
		forecaster->persistor->getStartDates(fxsrc->Symbol, fxsrc->TimeFrame, fxsrc->IsFilled, ds->sourceTS->date0, len, oDates);
		break;
	case FILE_SOURCE:
		filesrc = (sFileData*)ds->sourceTS->sourceData;
		forecaster->persistor->getStartDates(filesrc->srcFile, filesrc->fieldSep, filesrc->featuresCnt, ds->sourceTS->date0, len, oDates);
		break;
	case MT4_SOURCE:
		mt4src = (sMT4Data*)ds->sourceTS->sourceData;
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