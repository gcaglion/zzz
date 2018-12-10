
#include "sRoot.h"

//-- constructor / destructor
sRoot::sRoot(NativeReportProgress* progressReporter) : sCfgObj(nullptr, newsname("RootObj"), defaultdbg, progressReporter, nullptr, nullptr) {
	pid=GetCurrentProcessId();
	GUIreporter=progressReporter;
}
sRoot::~sRoot() {}

//-- core stuff
void sRoot::trainClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr) {


	//-- full filenames
	char clientffname[MAX_PATH];
	char shapeffname[MAX_PATH];
	char trainffname[MAX_PATH];
	char engineffname[MAX_PATH];

	char endtimeS[TIMER_ELAPSED_FORMAT_LEN];

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
		sCfg* clientCfg; safespawn(clientCfg, newsname("clientCfg"), erronlydbg, clientffname);
		sCfg* shapeCfg; safespawn(shapeCfg, newsname("shapeCfg"), erronlydbg, shapeffname);
		sCfg* trainCfg; safespawn(trainCfg, newsname("trainCfg"), erronlydbg, trainffname);
		sCfg* engCfg; safespawn(engCfg, newsname("engineCfg"), erronlydbg, engineffname);

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
		safespawn(trainDS, newsname("TrainDataSet"), erronlydbg, trainCfg, "/DataSet", shape->sampleLen, shape->predictionLen);
		safespawn(trainLog, newsname("TrainLogger"), erronlydbg, trainCfg, "/DataSet/Persistor");
		//-- 4. spawn engine the standard way
		safespawn(engine, newsname("TrainEngine"), erronlydbg, engCfg, "/Engine", shape->sampleLen*trainDS->selectedFeaturesCnt, shape->predictionLen*trainDS->selectedFeaturesCnt, pid);

		//-- training cycle core
		timer->start();
		//-- just load trainDS->TimeSerie; it should have its own date0 set already
		safecall(trainDS->sourceTS, load, TARGET, BASE);
		//-- do training (also populates datasets)
		safecall(engine, train, simulationId_, trainDS);
		//-- persist MSE logs
		safecall(engine, saveMSE);
		//-- persist Core logs
		safecall(engine, saveImage);
		//-- persist Engine Info
		safecall(engine, saveInfo);
		//-- Commit engine persistor data
		safecall(engine, commit);
		//-- stop timer, and save client info
		timer->stop(endtimeS);
		safecall(clientLog, saveClientInfo, pid, simulationId_, "Root.Tester", timer->startTime, timer->elapsedTime, trainDS->sourceTS->date0, "", "", true, false, clientffname, shapeffname, trainffname, engineffname);
		//-- Commit clientpersistor data
		safecall(clientLog, commit);

	}
	catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}


}
void sRoot::inferClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* inferXMLfile_, const char* engineXMLfile_, int savedEnginePid_, NativeReportProgress* progressPtr) {

	//-- full filenames
	char clientffname[MAX_PATH];
	char shapeffname[MAX_PATH];
	char inferffname[MAX_PATH];
	char engineffname[MAX_PATH];

	char endtimeS[TIMER_ELAPSED_FORMAT_LEN]; endtimeS[0]='\0';
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

		//-- 5.1 create client persistor, if needed
		bool saveClient;
		safecall(clientCfg, setKey, "/Client");
		safecall(clientCfg->currentKey, getParm, &saveClient, "saveClient");
		safespawn(clientLog, newsname("ClientLogger"), defaultdbg, clientCfg, "Persistor");

		//-- check for possible duplicate pid in db (through client persistor), and change it
		safecall(this, getSafePid, clientLog, &pid);

		//-- 2. spawn DataShape
		safespawn(shape, newsname("inferDataShape"), defaultdbg, shapeCfg, "/DataShape");
		//-- 3. spawn infer DataSet and its persistor
		safespawn(inferDS, newsname("inferDataSet"), defaultdbg, inferCfg, "/DataSet", shape->sampleLen, shape->predictionLen);
		safespawn(inferLog, newsname("inferLogger"), defaultdbg, inferCfg, "/DataSet/Persistor");
		//-- root-level persistor. this is used, among other things, to spawn engine by pid
		safespawn(engLog, newsname("ForecasterPersistor"), defaultdbg, engCfg, "/Engine/Persistor");
		//-- spawn engine from savedEnginePid_ with pid
		safespawn(engine, newsname("Engine"), defaultdbg, engCfg, "/Engine", shape->sampleLen*inferDS->selectedFeaturesCnt, shape->predictionLen*inferDS->selectedFeaturesCnt, engLog, pid, savedEnginePid_);

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
		safecall(clientLog, saveClientInfo, pid, simulationId_, "Root.Tester", timer->startTime, timer->elapsedTime, "", inferDS->sourceTS->date0, "", false, true, clientffname, shapeffname, inferffname, engineffname);
		//-- Commit clientpersistor data
		safecall(clientLog, commit);

	}
	catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}


}
void sRoot::bothClient(int simulationId_, const char* clientXMLfile_, const char* shapeXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr) {
	safecall(this, trainClient, simulationId_, clientXMLfile_, shapeXMLfile_, trainXMLfile_, engineXMLfile_, progressPtr);
	safecall(this, inferClient, simulationId_, clientXMLfile_, shapeXMLfile_, trainXMLfile_, engineXMLfile_, pid, progressPtr);
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
		root->inferClient(simulationId_, clientXMLfile_, shapeXMLfile_, inferXMLfile_, engineXMLfile_, savedEnginePid_, &progressPtr);
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
