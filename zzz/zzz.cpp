#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfg.h"
#include "../DataMgr/sData.h"
#include "../DataMgr/TimeSerie.h"
#include "../Forecaster/Forecaster.h"

numtype MyRndDbl(numtype min, numtype max) {
	unsigned int number;
	int err;
	numtype ret;

	err = rand_s(&number);
	ret = min+(numtype)number/((numtype)UINT_MAX+1) * (max-min);
	return ret;
}

struct sRoot : sObj {

	//-- 1. declarations
	sCfg* clientCfg;
	char cfgFileFullName[MAX_PATH];
	int cfgOverrideCnt=0;
	char cfgOverrideName[XMLLINE_MAXCNT][XMLKEY_PARM_NAME_MAXLEN];
	char cfgOverrideValS[XMLKEY_PARM_MAXCNT][XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];
	//--
	sData* forecastData;
	sTimeSerie* trainTS;
	sDataSet* trainDS;
	sFXData* fxData1;
	sDataShape* dshape1;
	sDataShape* dshape2;
	sForecaster* mainForecaster;
	sOraDB* oradb1;

	//tData*			fData=nullptr;		//-- Forecaster data
	//tEngine*		fEngine=nullptr;	//-- Forecaster engine
	//tLogger*		fPersistor=nullptr;	//-- Forecaster Persistor

	int sampleLen, PredictionLen, FeaturesCnt;
	int dsType;
	

	sRoot(int argc_=0, char* argv_[]=nullptr) : sObj(nullptr, newsname("RootObj"), nullptr) {
		dbg->pauseOnError=true;

		CLoverride(argc_, argv_);
	}
	~sRoot() {}

	void testDML() {
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

	void execute() {
		try {
			//-- 2. do stuff

			//testDML();

			safespawn(false, clientCfg, newsname("Root_Config"), defaultdbg, cfgFileFullName);
			safespawn(false, mainForecaster, newsname("Main_Forecaster"), defaultdbg, clientCfg, "/Forecaster");

		}
		catch (std::exception exc) {
			fail("Exception=%s", exc.what());
		}
	}

private:

	void CLoverride(int argc, char* argv[]) {
		char orName[XMLKEY_PARM_NAME_MAXLEN];
		char orValS[XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT];

		//-- set default cfgFilaName
		getFullFileName("../Client.xml", cfgFileFullName);

		for (int p=1; p<argc; p++) {
			if (!getValuePair(argv[p], &orName[0], &orValS[0], '=')) fail("wrong parameter format in command line: %s", argv[p]);
			if (_stricmp(orName, "--cfgFile")==0) {
				if (!getFullFileName(orValS, cfgFileFullName)) fail("could not set cfgFileFullName from override parameter: %s", orValS);
			} else {
				strcpy_s(cfgOverrideName[cfgOverrideCnt], XMLKEY_PARM_NAME_MAXLEN, orName);
				strcpy_s(cfgOverrideValS[cfgOverrideCnt], XMLKEY_PARM_VAL_MAXLEN*XMLKEY_PARM_VAL_MAXCNT, orValS);
				cfgOverrideCnt++;
			}
		}
	}

};

//-- client closure
#define terminate(success_, ...) { \
	int ret; \
	if(success_){ \
		printf("Client success. \n"); \
		ret = 0; \
	} else { \
		printf("Client failed: %s\n", __VA_ARGS__); \
		ret = -1; \
	} \
	printf("Press any key..."); getchar(); \
	return ret; \
}

int main(int argc, char* argv[]) {

	//-- 1. create root object. root constructor does everything else
	sRoot* root=nullptr;
	try {
		root=new sRoot(argc, argv);	//-- always takes default debugger settings
		root->execute();
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}

	terminate(true);
}