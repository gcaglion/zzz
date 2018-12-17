
#include "sRoot.h"
//#include <vld.h>

//-- constructor / destructor
sRoot::sRoot(NativeReportProgress* progressReporter) : sCfgObj(nullptr, newsname("RootObj"), defaultdbg, progressReporter, nullptr, nullptr) {
	pid=GetCurrentProcessId();
	GUIreporter=progressReporter;
}
sRoot::~sRoot() {}

//-- core stuff
void sRoot::trainClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr) {

	try {
		//-- 0. set full file name for each of the input files
		getFullPath(clientXMLfile_, clientffname);
		getFullPath(shapeXMLfile_, shapeffname);
		getFullPath(trainXMLfile_, trainffname);
		getFullPath(engineXMLfile_, engineffname);

		//-- 1. load separate sCfg* for client, dataShape, trainDataset, Engine
		safespawn(clientCfg, newsname("clientCfg"), erronlydbg, clientffname);
		safespawn(shapeCfg, newsname("shapeCfg"), erronlydbg, shapeffname);
		safespawn(trainCfg, newsname("trainCfg"), erronlydbg, trainffname);
		safespawn(engCfg, newsname("engineCfg"), erronlydbg, engineffname);

		//-- 5. create client persistor, if needed
		bool saveClient;
		safecall(clientCfg, setKey, "/Client");
		safecall(clientCfg->currentKey, getParm, &saveClient, "saveClient");
		safespawn(clientLog, newsname("ClientLogger"), erronlydbg, clientCfg, "Persistor");

		//-- check for possible duplicate pid in db (through client persistor), and change it
		safecall(this, getSafePid, clientLog, &pid);

		//-- 2. spawn DataShape
		safespawn(shape, newsname("TrainDataShape"), erronlydbg, shapeCfg, "/DataShape");
		//-- 3. spawn Train DataSet and its persistor
		safespawn(trainDS, newsname("TrainDataSet"), erronlydbg, trainCfg, "/DataSet", shape);
		safespawn(trainLog, newsname("TrainLogger"), erronlydbg, trainCfg, "/DataSet/Persistor");
		//-- 4. spawn engine the standard way
		safespawn(engine, newsname("TrainEngine"), erronlydbg, engCfg, "/Engine", shape, pid);

		//-- training cycle core
		timer->start();
		//-- just load trainDS->TimeSerie; it should have its own date0 set already
		safecall(trainDS->sourceTS, load, TARGET, BASE);
		//-- do training (also populates datasets)
		safecall(engine, train, simulationId_, trainDS);
		//-- persist MSE logs
		safecall(engine, saveMSE);
		//-- persist Core logs
		safecall(engine, saveCoreImages);
		safecall(engine, saveCoreLoggers);
		//-- persist Engine Info
		safecall(engine, saveInfo);
		//-- Commit engine persistor data
		safecall(engine, commit);
		//-- stop timer, and save client info
		timer->stop(endtimeS);
		safecall(clientLog, saveClientInfo, pid, simulationId_, "Root.Tester", timer->startTime, timer->elapsedTime, trainDS->sourceTS->date0, "", "", true, false, clientffname, shapeffname, trainffname, engineffname);
		//-- Commit clientpersistor data
		safecall(clientLog, commit);

		//-- cleanup
		delete engine;
		delete trainLog;
		delete trainDS;
		delete shape;
		delete clientLog;
		delete engCfg;
		delete trainCfg;
		delete shapeCfg;
		delete clientCfg;

	}
	catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}


}
void sRoot::inferClient(int simulationId_, const char* clientXMLfile_, const char* inferXMLfile_, int savedEnginePid_, NativeReportProgress* progressPtr) {

	try {
		//-- 0. set full file name for each of the input files
		getFullPath(clientXMLfile_, clientffname);
		getFullPath(inferXMLfile_, inferffname);

		//-- 1. load separate sCfg* for client, dataShape, inferDataset, Engine
		safespawn(clientCfg, newsname("clientCfg"), defaultdbg, clientffname);
		safespawn(inferCfg, newsname("inferCfg"), defaultdbg, inferffname);

		//-- 5.1 create client persistor, if needed
		bool saveClient;
		safecall(clientCfg, setKey, "/Client");
		safecall(clientCfg->currentKey, getParm, &saveClient, "saveClient");
		safespawn(clientLog, newsname("ClientLogger"), defaultdbg, clientCfg, "Persistor");

		//-- check for possible duplicate pid in db (through client persistor), and change it
		safecall(this, getSafePid, clientLog, &pid);

		//-- spawn engine from savedEnginePid_ with pid
		safespawn(engine, newsname("Engine"), defaultdbg, clientLog, pid, savedEnginePid_);

		//-- 2. shape is taken from from engine->shape
		shape=engine->shape;

		//-- 3. spawn infer DataSet and its persistor
		safespawn(inferDS, newsname("inferDataSet"), defaultdbg, inferCfg, "/DataSet", shape, shape->sampleLen);
		safespawn(inferLog, newsname("inferLogger"), defaultdbg, inferCfg, "/DataSet/Persistor");

		//-- core infer cycle
		timer->start();
		//-- set date0 in testDS->TimeSerie, and load it
		safecall(inferDS->sourceTS, load, TARGET, BASE);
		//-- do inference (also populates datasets)
		safecall(engine, infer, simulationId_, inferDS, savedEnginePid_);
		//-- persist Run logs
		safecall(engine, saveRun);
		//-- ommit engine persistor data
		safecall(engine, commit);
		//-- stop timer, and save client info
		timer->stop(endtimeS);
		safecall(clientLog, saveClientInfo, pid, simulationId_, "Root.Tester", timer->startTime, timer->elapsedTime, "", inferDS->sourceTS->date0, "", false, true, clientffname, "", inferffname, "");
		//-- Commit clientpersistor data
		safecall(clientLog, commit);

		//-- cleanup
		delete inferLog;
		delete inferDS;
		delete engine;
		delete clientLog;
		delete clientCfg;
		delete inferCfg;

	}
	catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}


}
void sRoot::bothClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr) {
	safecall(this, trainClient, simulationId_, clientXMLfile_, shapeXMLfile_, trainXMLfile_, engineXMLfile_, progressPtr);
	safecall(this, inferClient, simulationId_, clientXMLfile_, trainXMLfile_, pid, progressPtr);
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
void sRoot::getSafePid(sLogger* persistor, int* pid) {
	//-- look for pid in ClientInfo. if found, reduce by 1 until we find an unused pid
	bool found;
	do {
		safecall(persistor, findPid, (*pid), &found);
		if (found) (*pid)--;
	} while (found);

}

//-- temp stuff
#include "../CUDAwrapper/CUDAwrapper.h"

void sRoot::kaz() {

	sAlgebra* Alg1;
	safespawn(Alg1, newsname("%s_Algebra", name->base), defaultdbg);
	sOraData* oradb1;
	safespawn(oradb1, newsname("oradb1"), defaultdbg, "History", "HistoryPwd", "Algo");
	sOraData* oradb2;
	safespawn(oradb2, newsname("oradb1"), defaultdbg, "History", "HistoryPwd", "Algo");
	sFXDataSource* fxsrc1;
	safespawn(fxsrc1, newsname("FXDataSource1"), defaultdbg, oradb1, "EURUSD", "H1", false);
	sTimeSerie* ts1;
	safespawn(ts1, newsname("TimeSerie1"), defaultdbg, fxsrc1, "2017-10-20-00:00", 202, DT_DELTA, "c:/temp/DataDump");
	return;


	//======================================================================================
	sTimer* timer = new sTimer();
	char el[DATE_FORMAT_LEN];
	timer->start();
	Sleep(5000);
	timer->stop(el);

	sAlgebra* Alg=new sAlgebra(this, newsname("Alg1"), defaultdbg, nullptr);

	int vlen=5;
	numtype* v1d; Alg->myMalloc(&v1d, vlen);
	numtype* v2d; Alg->myMalloc(&v2d, vlen);
	numtype* v3d; Alg->myMalloc(&v3d, 1);
	numtype* v4d; Alg->myMalloc(&v4d, vlen);
	//--
	numtype* v1h=(numtype*)malloc(vlen*sizeof(numtype));
	numtype* v2h=(numtype*)malloc(vlen*sizeof(numtype));
	numtype* v3h=(numtype*)malloc(1*sizeof(numtype));
	numtype* v4h=(numtype*)malloc(vlen*sizeof(numtype));
	//--
	Alg->Vinit(vlen, v1d, (numtype)(vlen/2), (numtype)1);
	Alg->Vinit(vlen, v2d, (numtype)(vlen/2), (numtype)-1);
	Alg->Vinit(vlen, v4d, (numtype)(vlen/2), (numtype)-1);
	//--
	Alg->d2h(v1h, v1d, vlen*sizeof(numtype), false);
	Alg->d2h(v2h, v2d, vlen*sizeof(numtype), false);
	Alg->d2h(v4h, v4d, vlen*sizeof(numtype), false);
	//--

	//--------------- Vssum ---------------
	Vssum_cu(Alg->cublasH, vlen, v1d, v3h);
	printf("Vssum(v1)-GPU = %f\n", (*v3h));
	(*v3h)=0;
	for (int i=0; i<vlen; i++) (*v3h)+=v1h[i]*v1h[i];
	printf("Vssum(v1)-CPU = %f\n", (*v3h));
	//-------------------------------------

	//--------------- Vnorm --------------
	Vnorm_cu(Alg->cublasH, vlen, v1d, v3h);
	printf("Vsnorm(v1)-GPU = %f\n", (*v3h));
	(*v3h)=0;
	for (int i=0; i<vlen; i++) (*v3h)+=v1h[i]*v1h[i];
	(*v3h)=sqrt(*v3h);
	printf("Vsnorm(v1)-CPU = %f\n", (*v3h));
	//-------------------------------------
	system("pause");
	return;

	//---------------- Vadd with scale (v1*1.2 + v2*-0.5 = v4), followed by Vnorm ----------
	Vadd_cu(vlen, v1d, 1.2f, v2d, -0.5f, v4d);
	Vnorm_cu(Alg->cublasH, vlen, v4d, v3h);
	printf("Vadd(v1*1.2+v2*-0.5)-GPU = %f\n", (*v3h));

	for (int i=0; i<vlen; i++) v4h[i]=v1h[i]*1.2f-v2h[i]*0.5f;
	(*v3h)=0;
	for (int i=0; i<vlen; i++) (*v3h)+=v4h[i]*v4h[i];
	(*v3h)=sqrt((*v3h));
	printf("Vadd(v1*1.2+v2*-0.5)-CPU = %f\n", (*v3h));
	//---------------------------------------------------------------------------------------

	system("pause");
	return;

	//------------------------------------------------------
	Vscale_cu(vlen, v1d, 0.1f);
	for (int i=0; i<vlen; i++) v1h[i]=v1h[i]*0.1f;
	//------------------------------------------------------


	//------------------------------------------------------
	Vnorm_cu(Alg->cublasH, vlen, v1d, v3h);
	//------------------------------------------------------
	printf("Vnorm(v1)-GPU = %f\n", (*v3h));

	(*v3h)=0;
	//------------------------------------------------------
	for (int i=0; i<vlen; i++) (*v3h)+=v1h[i]*v1h[i];
	(*v3h)=sqrt((*v3h));
	//------------------------------------------------------
	printf("Vnorm(v1)-CPU = %f\n", (*v3h));


	//------------------------------------------------------
	VdotV_cu(Alg->cublasH, vlen, v1d, v2d, v3h);
	//------------------------------------------------------
	printf("Vdotv(v1,v2)-GPU = %f\n", (*v3h));

	(*v3h)=0;
	//------------------------------------------------------
	for (int i = 0; i < vlen; i++) (*v3h) += v1h[i]*v2h[i];
	printf("Vdotv(v1,v2)-CPU = %f\n", (*v3h));

	system("pause");

/*
//	sCfg* ds2Cfg=new sCfg(this, newsname("ds2Cfg"), defaultdbg, "Config/Light/Infer.xml");
//	sDataSet* ds2=new sDataSet(this, newsname("ds2"), defaultdbg, ds2Cfg, "DataSet", 100, 3);

	sOraData* oradb1=new sOraData(this, newsname("oradb1"), defaultdbg, "History", "HistoryPwd", "Algo");
	sFXDataSource* fxsrc1=new sFXDataSource(this, newsname("FXDataSource1"), defaultdbg, oradb1, "EURUSD", "H1", false);
	sTimeSerie* ts1 = new sTimeSerie(this, newsname("TimeSerie1"), defaultdbg, fxsrc1, "2017-10-20-00:00", 202, DT_DELTA, "c:/temp/DataDump");

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
*/
}

//-- GUI hooks
extern "C" __declspec(dllexport) int _trainClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress progressPtr) {
	sRoot* root=nullptr;
	try {
		root=new sRoot(&progressPtr);
		sdp progressVar; progressVar.p1=10; progressVar.p2=50.0f; strcpy_s(progressVar.msg, DBG_MSG_MAXLEN, "Starting Train ...\n");
		progressPtr(10, progressVar.msg);
		root->trainClient(simulationId_, clientXMLfile_, shapeXMLfile_, trainXMLfile_, engineXMLfile_, &progressPtr);
	} catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}
	terminate(true, "");
}
extern "C" __declspec(dllexport) int _inferClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* inferXMLfile_, const char* engineXMLfile_, int savedEnginePid_, NativeReportProgress progressPtr) {
	sRoot* root=nullptr;
	try {
		root=new sRoot(&progressPtr);
		sdp progressVar; progressVar.p1=10; progressVar.p2=50.0f; strcpy_s(progressVar.msg, DBG_MSG_MAXLEN, "Starting Infer ...\n");
		progressPtr(10, progressVar.msg);
		root->inferClient(simulationId_, clientXMLfile_, inferXMLfile_, savedEnginePid_, &progressPtr);
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}
	terminate(true, "");
}
extern "C" __declspec(dllexport) int _bothClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress progressPtr) {

	sRoot* root=nullptr;
	try {
		root=new sRoot(&progressPtr);
		sdp progressVar; progressVar.p1=10; progressVar.p2=50.0f; strcpy_s(progressVar.msg, DBG_MSG_MAXLEN, "Starting Train + Infer ...\n");
		progressPtr(10, progressVar.msg);
		root->bothClient(simulationId_, clientXMLfile_, shapeXMLfile_, trainXMLfile_, engineXMLfile_, &progressPtr);
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}
	terminate(true, "");
}

//-- MT4 stuff
void sRoot::getForecast(numtype* iBarO, numtype* iBarH, numtype* iBarL, numtype* iBarC, numtype* iBarV, numtype* oForecastH, numtype* oForecastL) {
	for (int b=0; b<MT4predictionLen; b++) {
		oForecastH[b]=(numtype)b/10;
		oForecastL[b]=(numtype)b/20;
	}
}
void sRoot::setMT4env(int accountId_, int* oSampleLen_, int* oPredictionLen_) {
	MT4accountId=accountId_;

	//-- these would be set by engine creation
	MT4sampleLen=20;
	MT4predictionLen=3;
	//------------------------------------------

	//-- return them to caller
	(*oSampleLen_)=MT4sampleLen;
	(*oPredictionLen_)=MT4predictionLen;

}
extern "C" __declspec(dllexport) int _createEnv(int accountId_, char* oEnvS, int* oSampleLen_, int* oPredictionLen_) {

	static sRoot* root;
	try {
		root=new sRoot(nullptr);
		sprintf_s(oEnvS, 64, "%p", root);
		root->setMT4env(accountId_, oSampleLen_, oPredictionLen_);
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}


	return 0;

}
extern "C" __declspec(dllexport) int _getForecast(void* iEnv, numtype* iBarO, numtype* iBarH, numtype* iBarL, numtype* iBarC, numtype* iBarV, numtype* oForecastH, numtype* oForecastL) {

	try {
		((sRoot*)iEnv)->getForecast(iBarO, iBarH, iBarL, iBarC, iBarV, oForecastH, oForecastL);
	}
	catch (std::exception exc) {
		return -1;
	}

	return 0;
}
extern "C" __declspec(dllexport) int _destroyEnv(void* iEnv) {
	sRoot* root=(sRoot*)iEnv;
	delete root;
	return 0;
}
extern "C" __declspec(dllexport) int _dioPorco(int i1, char* oEnvS, int* o1) {
	(*o1)=i1*2;
	char* dp="DioPorco!";
	sprintf_s(oEnvS, 64, "%s", dp);
	return 99;
}
