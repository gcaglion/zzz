#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfg.h"
#include "../DataMgr/sData.h"
#include "../DataMgr/TimeSerie.h"
#include "../Forecaster/Forecaster.h"

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
	sOraConnection* dbconn1;
	sDataShape* dshape1;
	sDataShape* dshape2;
	sForecaster* mainForecaster=nullptr;

	//tData*			fData=nullptr;		//-- Forecaster data
	//tEngine*		fEngine=nullptr;	//-- Forecaster engine
	//tLogger*		fPersistor=nullptr;	//-- Forecaster Persistor

	int sampleLen, PredictionLen, FeaturesCnt;
	int dsType;
	

	sRoot(int argc_=0, char* argv_[]=nullptr) : sObj(nullptr, newsname("RootObj"), nullptr) {
		dbg->pauseOnError=true;

		CLoverride(argc_, argv_);
	}

	void execute() {
		try {
			//-- 2. do stuff
			//safespawn(clientCfg, newsname("client_Config"), nullptr, "C:\\Users\\gcaglion\\dev\\zzz\\ForecastData.xml");
			safespawn(clientCfg, newsname("Root_Config"), defaultdbg, cfgFileFullName);

			//safespawn(mainForecaster, newsname("Main_Forecaster"), defaultdbg, clientCfg, "/Forecaster");
			sName* n=new sName("dshape1");
			dshape1=new sDataShape(this, n, nullptr, clientCfg, "/Forecaster/Data/Shape");
			//safespawn(dshape2, newsname("Shape2"), nullptr, clientCfg, "/Forecaster/Data/Shape");
			//safespawn(forecastData, newsname("Main_Forecaster_Data"), defaultdbg, clientCfg, "/Forecaster/Data");

			//safespawn(dbconn1, newsname("DBConn1"), nullptr, "History", "HistoryPwd", "Algo");
			//safespawn(dbconn1, newsname("DBConn1"), defaultdbg, clientCfg, "/Forecaster/Data/Train/Dataset/TimeSerie/FXDB_DataSource/DBConnection");
			//safecall(clientCfg, setKey, "Forecaster");
			//safespawn(forecastData, newsname("ForecastData"), defaultdbg, clientCfg, "/Forecaster/Data");
			//safespawn(dshape1, newsname("Shape1"), nullptr, clientCfg, "Data/Shape");

/*			safecall(clientCfg, setKey, "Forecaster/Data/Train/Dataset");
			safecall(clientCfg, setKey, "/Forecaster/Data/Train/Dataset");
			safecall(clientCfg, setKey, "///Forecaster/Data/Train/Dataset");
			safecall(clientCfg, setKey, "../");
			safecall(clientCfg, setKey, "Dataset");
			safecall(clientCfg, setKey, "../../Shape");

			safecall(clientCfg, getParm, &sampleLen, "SampleLen");
			safecall(clientCfg, getParm, &PredictionLen, "PredictionLen");
			safecall(clientCfg, getParm, &FeaturesCnt, "FeaturesCount");

			safespawn(forecastData, newsname("Forecaster_Data"), nullptr, clientCfg, "/Forecaster/Data");

			//safespawn(fxData1, newsname("fxData1"), nullptr, clientCfg, "FXDB_DataSource");

			//safespawn(trainTS, newsname("TrainingTimeSerie"), nullptr, clientCfg, "TimeSerie");
			
			safecall(clientCfg, setKey, "../Train/Dataset");
			safecall(clientCfg, setKey, "TimeSerie/DataSource");
			safecall(clientCfg, getParm, &dsType, "Type");

			safecall(clientCfg, setKey, "../Test/Dataset");

			safespawn(XMLparms, tParmsSource, "C:\\Users\\gcaglion\\dev\\cudaNN\\Client\\Client.xml", argc, argv, true);
			//-- 2. create Forecaster Data from parms
			safespawn(fData, tData, XMLparms, ".Forecaster.Data");
			//-- 3. create Forecaster Engine from parms
			safespawn(fEngine, tEngine, XMLparms, ".Forecaster.Engine", fData->shape);
			//-- 4. create Forecaster Persistor
			safespawn(fPersistor, tLogger, XMLparms, ".Forecaster.Persistor");
*/
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
		getFullFileName("Client.xml", cfgFileFullName);

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

//-- client closures
#define clientFail(failmsg){ \
	delete root; \
	printf("Client failed: %s\n", failmsg); \
	system("pause"); \
	return -1; \
}
#define clientSuccess(){ \
	delete root; \
	printf("Client success. \n"); \
	system("pause"); \
	return 0; \
}


int main(int argc, char* argv[]) {

	int p1=1;
	char* p2="aaa";


	//-- 1. create root object. root constructor does everything else
	sRoot* root=nullptr;
	try {
		root=new sRoot(argc, argv);	//-- always takes default debugger settings
		root->execute();
	}
	catch (std::exception exc) {
		clientFail("Exception thrown by root. See stack.");
	}

	clientSuccess();


	return 0;
}