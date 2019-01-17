
#include "sRoot.h"
//#include <vld.h>

//-- constructor / destructor
sRoot::sRoot(NativeReportProgress* progressReporter) : sCfgObj(nullptr, newsname("RootObj"), defaultdbg, progressReporter, nullptr, nullptr) {
	pid=GetCurrentProcessId();
	GUIreporter=progressReporter;
}
sRoot::~sRoot() {}

//-- core stuff
void sRoot::trainClient(int simulationId_, const char* clientXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr) {

	try {
		//-- 0. set full file name for each of the input files
		getFullPath(clientXMLfile_, clientffname);
		getFullPath(trainXMLfile_, trainffname);
		getFullPath(engineXMLfile_, engineffname);

		//-- 1. load separate sCfg* for client, trainDataset, Engine
		safespawn(clientCfg, newsname("clientCfg"), erronlydbg, clientffname);
		safespawn(trainCfg, newsname("trainCfg"), erronlydbg, trainffname);
		safespawn(engCfg, newsname("engineCfg"), erronlydbg, engineffname);

		//-- 5. create client persistor, if needed
		bool saveClient;
		safecall(clientCfg, setKey, "/Client");
		safecall(clientCfg->currentKey, getParm, &saveClient, "saveClient");
		safespawn(clientLog, newsname("ClientLogger"), defaultdbg, clientCfg, "Persistor");

		//-- check for possible duplicate pid in db (through client persistor), and change it
		safecall(this, getSafePid, clientLog, &pid);

		//-- 3. spawn Train DataSet and its persistor
		safespawn(trainDS, newsname("TrainDataSet"), defaultdbg, trainCfg, "/DataSet", false);
		safespawn(trainLog, newsname("TrainLogger"), defaultdbg, trainCfg, "/DataSet/Persistor");
		//-- 4. spawn engine the standard way
		safespawn(engine, newsname("TrainEngine"), defaultdbg, engCfg, "/Engine", trainDS->shape, pid);

		//-- training cycle core
		timer->start();
		//-- do training (also populates datasets)
		safecall(engine, train, simulationId_, trainDS);

		//-- persist MSE logs
		safecall(engine, saveMSE);
		//-- persist Core logs
		safecall(engine, saveCoreImages);
		safecall(engine, saveCoreLoggers);
		//-- persist Engine Info
		safecall(engine, saveInfo);

		//-- do infer on training data, without reloading engine
		safecall(engine, infer, simulationId_, trainDS, pid);
		//-- persist Run logs
		safecall(engine, saveRun);

		//-- Commit engine persistor data
		safecall(engine, commit);
		//-- stop timer, and save client info
		timer->stop(endtimeS);
		safecall(clientLog, saveClientInfo, pid, simulationId_, "Root.Tester", timer->startTime, timer->elapsedTime, trainDS->sourceTS[0]->date0, "", "", true, false, clientffname, "", trainffname, engineffname);
		//-- persist XML config parameters for Client,DataSet,Engine
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 0, clientCfg);
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 2, trainCfg);
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 3, engCfg);
		//-- Commit clientpersistor data
		safecall(clientLog, commit);

		//-- cleanup
		delete engine;
		delete trainLog;
		delete trainDS;
		delete clientLog;
		delete engCfg;
		delete trainCfg;
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

		//-- 1. load separate sCfg* for client, inferDataset, Engine
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
		
		//-- 3. spawn infer DataSet and its persistor
		safespawn(inferDS, newsname("inferDataSet"), defaultdbg, inferCfg, "/DataSet", true);
		safespawn(inferLog, newsname("inferLogger"), defaultdbg, inferCfg, "/DataSet/Persistor");

		//-- core infer cycle
		timer->start();
		//-- do inference (also populates datasets)
		safecall(engine, infer, simulationId_, inferDS, savedEnginePid_);
		//-- persist Run logs
		safecall(engine, saveRun);
		//-- ommit engine persistor data
		safecall(engine, commit);
		//-- stop timer, and save client info
		timer->stop(endtimeS);
		safecall(clientLog, saveClientInfo, pid, simulationId_, "Root.Tester", timer->startTime, timer->elapsedTime, "", inferDS->sourceTS[0]->date0, "", false, true, clientffname, "", inferffname, "");
		//-- persist XML config parameters for Client,DataSet,Engine
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 0, clientCfg);
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 2, inferCfg);
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

//-- utils stuff
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

void mallocBars(int sampleLen, int predictionLen, long** barT, double** barO, double** barH, double** barL, double** barC, double** barV, double** forecastO, double** forecastH, double** forecastL, double** forecastC, double** forecastV){
	(*barT)=(long*)malloc(sampleLen*sizeof(long));
	(*barO)=(double*)malloc(sampleLen*sizeof(double));
	(*barH)=(double*)malloc(sampleLen*sizeof(double));
	(*barL)=(double*)malloc(sampleLen*sizeof(double));
	(*barC)=(double*)malloc(sampleLen*sizeof(double));
	(*barV)=(double*)malloc(sampleLen*sizeof(double));
	//--
	(*forecastO)=(double*)malloc(predictionLen*sizeof(double));
	(*forecastH)=(double*)malloc(predictionLen*sizeof(double));
	(*forecastL)=(double*)malloc(predictionLen*sizeof(double));
	(*forecastC)=(double*)malloc(predictionLen*sizeof(double));
	(*forecastV)=(double*)malloc(predictionLen*sizeof(double));

	//-- ... set barO/H/L/C/V
	for (int i=0; i<sampleLen; i++) {
		(*barT)[i]=(i+1)*3600;
		(*barO)[i]=1.3200;
		(*barH)[i]=1.3400;
		(*barL)[i]=1.3100;
		(*barC)[i]=1.3300;
		(*barV)[i]=-1;
	}
}
void sRoot::kaz() {

	char ffname[MAX_PATH];
	char* BaseDataSetXMLFile="Config/30/DataSets/base.xml"; getFullPath(BaseDataSetXMLFile, ffname);
	int simulationId=99;
	int pid=GetCurrentProcessId();
	int tid=GetCurrentThreadId();

	//-- need client config to create a client persistor
	char* clientXMLfile="Config/master/99/Client.xml"; getFullPath(clientXMLfile, clientffname);
	sCfg* clientCfg; safespawn(clientCfg, newsname("clientConfig"), defaultdbg, clientXMLfile);
	sLogger* clientPersistor= new sLogger(this, newsname("clientLogger"), defaultdbg, GUIreporter, clientCfg, "/Client/Persistor");

	char featList[XMLKEY_PARM_VAL_MAXLEN]; char* tmpS=&featList[0]; int s=0;
	clientCfg->setKey("/Client/MetaTrader/Chart0");
	safecall(clientCfg->currentKey, getParm, &tmpS, "TimeFrame");
	safecall(clientCfg->currentKey, getParm, &tmpS, "SelectedFeatures");


	return;
/*
	int accountId=25307435;
	int enginePid=3724;
	bool useVolume=false;
	int dt=1;

	//-- need client config to create a client persistor
	char* clientXMLfile="C:/Users/gcaglion/dev/zzz/Config.NoInclude/Client.xml"; getFullPath(clientXMLfile, clientffname);
	sCfg* clientCfg; safespawn(clientCfg, newsname("clientConfig"), defaultdbg, clientXMLfile);
	sLogger* clientPersistor= new sLogger(this, newsname("clientLogger"), defaultdbg, GUIreporter, clientCfg, "/Client/Persistor");

	//-- avoid duplicate client pid
	int clientPid=GetCurrentProcessId();
	safecall(this, getSafePid, clientPersistor, &clientPid);

	// spawn engine from enginePid
	sEngine* mt4eng= new sEngine(this, newsname("Engine_%d", pid), defaultdbg, GUIreporter, clientPersistor, clientPid, enginePid);

	//-- simulate MT bars
	long* barT=nullptr; double* barO; double* barH; double* barL; double* barC; double* barV; double* forecastO; double* forecastH; double* forecastL; double* forecastC; double* forecastV;
	long baseBarT=0; double baseBarO=0, baseBarH=0, baseBarL=0, baseBarC=0, baseBarV=0;
	mallocBars(mt4eng->shape->sampleLen, mt4eng->shape->predictionLen, &barT, &barO, &barH, &barL, &barC, &barV, &forecastO, &forecastH, &forecastL, &forecastC, &forecastV);
	//-------------------------

	int selF[5];
	int selFcnt=(useVolume) ? 5 : 4;
	for (int f=0; f<selFcnt; f++) selF[f]=f;

	sDataShape* mtDataShape= new sDataShape(this, newsname("MT4DataShape"), defaultdbg, nullptr, mt4eng->shape->sampleLen, mt4eng->shape->predictionLen, selFcnt);
	sMT4DataSource* mtDataSrc= new sMT4DataSource(this, newsname("MT4DataSource"), defaultdbg, nullptr, mt4eng->shape->sampleLen, barT, barO, barH, barL, barC, barV, baseBarT, baseBarO, baseBarH, baseBarL, baseBarC, baseBarV);
	sDataSet* mtDataSet= new sDataSet(this, newsname("MTdataSet"), defaultdbg, nullptr, mtDataShape, mtDataSrc, selFcnt, selF, dt, true, "C:/temp/DataDump");

	mtDataSet->sourceTS->load(ACTUAL, BASE);

	//-- do inference (also populates datasets)
	safecall(mt4eng, infer, accountId, mtDataSet, enginePid);

	//-- prediction is in dataset->predictionSBF
	for (int b=0; b<mt4eng->shape->predictionLen; b++) {
		forecastO[b]=mtDataSet->predictionSBF[b*selFcnt+FXOPEN];
		forecastH[b]=mtDataSet->predictionSBF[b*selFcnt+FXHIGH];
		forecastL[b]=mtDataSet->predictionSBF[b*selFcnt+FXLOW];
		forecastC[b]=mtDataSet->predictionSBF[b*selFcnt+FXCLOSE];
		if(useVolume) forecastV[b]=mtDataSet->predictionSBF[b*selFcnt+FXVOLUME];
	}
	

	//-- stop timer, and save client info
	timer->stop(endtimeS);
	safecall(clientPersistor, saveClientInfo, clientPid, accountId, "Root.kaz", timer->startTime, timer->elapsedTime, "", mtDataSet->sourceTS->date0, "", false, true, clientffname, "", "MT4", "");
	//-- Commit clientpersistor data
	safecall(clientPersistor, commit);

	return;

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
*/
/*
//	sCfg* ds2Cfg=new sCfg(this, newsname("ds2Cfg"), defaultdbg, "Config/Light/Infer.xml");
//	sDataSet* ds2=new sDataSet(this, newsname("ds2"), defaultdbg, ds2Cfg, "DataSet", 100, 3);

	sOraData* oradb1=new sOraData(this, newsname("oradb1"), defaultdbg, "History", "HistoryPwd", "Algo");
	sFXDataSource* fxsrc1=new sFXDataSource(this, newsname("FXDataSource1"), defaultdbg, oradb1, "EURUSD", "H1", false);
	sTimeSerie* ts1 = new sTimeSerie(this, newsname("TimeSerie1"), defaultdbg, fxsrc1, "2017-10-20-00:00", 202, DT_DELTA, "c:/temp/DataDump");

	const int selFcnt=4; int selF[selFcnt]={ 0,1,2,3 };
	int sampleLen=10; int predictionLen=3;
	sDataSet* ds1= new sDataSet(this, newsname("DataSet1"), defaultdbg, ts1, sampleLen, predictionLen, 10, selFcnt, selF, false, "C:/Temp/DataDump");

	ts1->load(ACTUAL, BASE, "201710010000");
	ts1->dump(ACTUAL, BASE);

	ts1->transform(ACTUAL, DT_DELTA);
	ts1->dump(ACTUAL, TR);

	ts1->scale(ACTUAL, TR, -1, 1);
	ts1->dump(ACTUAL, TRS);

	//-- Engine work... copy ACTUAL_TRS->PREDICTED_TRS, AND SKIP FIRST SAMPLE!
	int idx;
	for (int s=0; s<ts1->stepsCnt; s++) {
		for (int f=0; f<ts1->sourceData->featuresCnt; f++) {
			idx=s*ts1->sourceData->featuresCnt+f;
			ts1->val[PREDICTED][TRS][idx]=ts1->val[ACTUAL][TRS][idx];
		}
	}
	ts1->dump(ACTUAL, TRS);
	ts1->dump(PREDICTED, TRS);

	ts1->unscale(PREDICTED, -1, 1, selFcnt, selF, sampleLen );
	ts1->dump(PREDICTED, TR);

	ts1->untransform(PREDICTED, PREDICTED, sampleLen, selFcnt, selF);
	ts1->dump(PREDICTED, BASE);
*/
}

//-- GUI hooks
extern "C" __declspec(dllexport) int _trainClient(int simulationId_, const char* clientXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress progressPtr) {
	sRoot* root=nullptr;
	try {
		root=new sRoot(&progressPtr);
		sdp progressVar; progressVar.p1=10; progressVar.p2=50.0f; strcpy_s(progressVar.msg, DBG_MSG_MAXLEN, "Starting Train ...\n");
		progressPtr(10, progressVar.msg);
		root->trainClient(simulationId_, clientXMLfile_, trainXMLfile_, engineXMLfile_, &progressPtr);
	} catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}
	terminate(true, "");
}
extern "C" __declspec(dllexport) int _inferClient(int simulationId_, const char* clientXMLfile_, const char* inferXMLfile_, const char* engineXMLfile_, int savedEnginePid_, NativeReportProgress progressPtr) {
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

//-- MT4 stuff

#define MT_MAX_SERIES_CNT 12
void sRoot::getSeriesInfo(int* oSeriesCnt_, int* oSampleLen_, int* oPredictionLen_, char* oSymbolsCSL_, char* oTimeFramesCSL_, char* oFeaturesCSL_) {

	char tmpSymbol[XMLKEY_PARM_VAL_MAXLEN]; char tmpSymbolList[XMLKEY_PARM_VAL_MAXLEN]="";
	char tmpTF[XMLKEY_PARM_VAL_MAXLEN]; char tmpTFList[XMLKEY_PARM_VAL_MAXLEN]="";
	char* tmpC;

	int tmpFeaturesCnt; int tmpFeature[MAX_DATA_FEATURES]; char tmpFeatureS[XMLKEY_PARM_VAL_MAXLEN]; char tmpFeaturesList[XMLKEY_PARM_VAL_MAXLEN]="";
	int* tmpFeatureP=&tmpFeature[0];

	safecall(MT4clientCfg->currentKey, getParm, oSeriesCnt_, "MetaTrader/ChartsCount");
	for (int s=0; s<(*oSeriesCnt_); s++) {
		safecall(MT4clientCfg->currentKey, getParm, &oSampleLen_[s], (newsname("MetaTrader/Chart%d/SampleLen", s))->base);
		safecall(MT4clientCfg->currentKey, getParm, &oPredictionLen_[s], (newsname("MetaTrader/Chart%d/PredictionLen", s))->base);
		//--

		tmpC=&tmpSymbol[0]; safecall(MT4clientCfg->currentKey, getParm, &tmpC, (newsname("MetaTrader/Chart%d/Symbol", s))->base);
		tmpC=&tmpTF[0]; safecall(MT4clientCfg->currentKey, getParm, &tmpC, (newsname("MetaTrader/Chart%d/TimeFrame", s))->base);
		safecall(MT4clientCfg->currentKey, getParm, &tmpFeatureP, (newsname("MetaTrader/Chart%d/SelectedFeatures", s))->base, false, &tmpFeaturesCnt);
		//-- build symbol and TF CSLs
		strcat_s(tmpSymbolList, XMLKEY_PARM_VAL_MAXLEN, tmpSymbol); if (s<((*oSeriesCnt_)-1)) strcat_s(tmpSymbolList, XMLKEY_PARM_VAL_MAXLEN, "|");
		strcat_s(tmpTFList, XMLKEY_PARM_VAL_MAXLEN, tmpTF); if (s<((*oSeriesCnt_)-1)) strcat_s(tmpTFList, XMLKEY_PARM_VAL_MAXLEN, "|");
		for (int f=0; f<tmpFeaturesCnt; f++) {
			sprintf_s(tmpFeatureS, XMLKEY_PARM_VAL_MAXLEN, "%d", tmpFeature[f]);
			strcat_s(tmpFeaturesList, XMLKEY_PARM_VAL_MAXLEN, tmpFeatureS); if (f<(tmpFeaturesCnt-1)) strcat_s(tmpFeaturesList, XMLKEY_PARM_VAL_MAXLEN, ",");
		}
		if (s<((*oSeriesCnt_)-1)) strcat_s(tmpFeaturesList, XMLKEY_PARM_VAL_MAXLEN, "|");
	}
	strcpy_s(oSymbolsCSL_, XMLKEY_PARM_VAL_MAXLEN, tmpSymbolList);
	strcpy_s(oTimeFramesCSL_, XMLKEY_PARM_VAL_MAXLEN, tmpTFList);
	strcpy_s(oFeaturesCSL_, XMLKEY_PARM_VAL_MAXLEN, tmpFeaturesList);

}
void sRoot::getForecast(long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, long iBaseBarT, double iBaseBarO, double iBaseBarH, double iBaseBarL, double iBaseBarC, double iBaseBarV, double* oForecastO, double* oForecastH, double* oForecastL, double* oForecastC, double* oForecastV) {

	int selF[5];
	int selFcnt=(MT4useVolume) ? 5 : 4;
	for (int f=0; f<selFcnt; f++) selF[f]=f;

	sMT4DataSource* mtDataSrc; safespawn(mtDataSrc, newsname("MT4DataSource"), defaultdbg, MT4engine->shape->sampleLen, iBarT, iBarO, iBarH, iBarL, iBarC, iBarV, iBaseBarT, iBaseBarO, iBaseBarH, iBaseBarL, iBaseBarC, iBaseBarV);
	sDataSet* mtDataSet; safespawn(mtDataSet, newsname("MTdataSet"), defaultdbg, MT4engine->shape, mtDataSrc, selFcnt, selF, MT4dt, MT4doDump);

	mtDataSet->sourceTS[0]->load(ACTUAL, BASE);

	//-- do inference (also populates datasets)
	safecall(MT4engine, infer, MT4accountId, mtDataSet, MT4enginePid);
	
	//-- prediction is in dataset->predictionSBF

	//-- unscale
	for (int b=0; b<MT4engine->shape->predictionLen; b++) {
		oForecastO[b]=(mtDataSet->predictionSBF[b*selFcnt+FXOPEN]-mtDataSet->sourceTS[0]->scaleP[FXOPEN])/mtDataSet->sourceTS[0]->scaleM[FXOPEN];
		oForecastH[b]=(mtDataSet->predictionSBF[b*selFcnt+FXHIGH]-mtDataSet->sourceTS[0]->scaleP[FXHIGH])/mtDataSet->sourceTS[0]->scaleM[FXHIGH];
		oForecastL[b]=(mtDataSet->predictionSBF[b*selFcnt+FXLOW]-mtDataSet->sourceTS[0]->scaleP[FXLOW])/mtDataSet->sourceTS[0]->scaleM[FXLOW];
		oForecastC[b]=(mtDataSet->predictionSBF[b*selFcnt+FXCLOSE]-mtDataSet->sourceTS[0]->scaleP[FXCLOSE])/mtDataSet->sourceTS[0]->scaleM[FXCLOSE];
		if (MT4useVolume) oForecastV[b]=(mtDataSet->predictionSBF[b*selFcnt+FXVOLUME]-mtDataSet->sourceTS[0]->scaleP[FXVOLUME])/mtDataSet->sourceTS[0]->scaleM[FXVOLUME];
	}
	for (int b=0; b<MT4engine->shape->predictionLen; b++) info("TR   oForecastO[%d]=%f ; oForecastH[%d]=%f ; oForecastL[%d]=%f ; oForecastC[%d]=%f", b, oForecastO[b], b, oForecastH[b], b, oForecastL[b], b, oForecastC[b]);

	//-- untransform. baseVal is taken from bar0
	if (MT4dt==DT_DELTA) {
		oForecastO[0]=iBarO[MT4engine->shape->sampleLen-1]+oForecastO[0];
		oForecastH[0]=iBarH[MT4engine->shape->sampleLen-1]+oForecastH[0];
		oForecastL[0]=iBarL[MT4engine->shape->sampleLen-1]+oForecastL[0];
		oForecastC[0]=iBarC[MT4engine->shape->sampleLen-1]+oForecastC[0];
		oForecastV[0]=iBarV[MT4engine->shape->sampleLen-1]+oForecastV[0];
		for (int b=0; b<MT4engine->shape->predictionLen; b++){
			//-- next steps
		}
	}
	for (int b=0; b<MT4engine->shape->predictionLen; b++) info("BASE oForecastO[%d]=%f ; oForecastH[%d]=%f ; oForecastL[%d]=%f ; oForecastC[%d]=%f", b, oForecastO[b], b, oForecastH[b], b, oForecastL[b], b, oForecastC[b]);
	
	//-- stop timer, and save client info
	timer->stop(endtimeS);

//	safecall(MT4clientLog, saveClientInfo, MT4clientPid, MT4accountId, "Root.kaz", timer->startTime, timer->elapsedTime, "", mtDataSet->sourceTS->date0, "", false, true, clientffname, "", "MT4", "");
	//-- Commit clientpersistor data
//	safecall(MT4clientLog, commit);

	//-- free(s)
	delete mtDataSrc;
	delete mtDataSet;

}
void sRoot::saveTradeInfo(int iPositionTicket, char* iPositionOpenTime, char* iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult) {
	safecall(MT4clientLog, saveTradeInfo, MT4clientPid, MT4sessionId, MT4accountId, MT4enginePid, iPositionTicket, iPositionOpenTime, iLastBarT, iLastBarO, iLastBarH, iLastBarL, iLastBarC, iLastBarV, iForecastO, iForecastH, iForecastL, iForecastC, iForecastV, iTradeScenario, iTradeResult);
}
void sRoot::setMT4env(int clientPid_, int accountId_, char* clientXMLFile_, int savedEnginePid_, bool useVolume_, int dt_, bool doDump_) {
	MT4clientPid=clientPid_;
	MT4accountId=accountId_;
	MT4enginePid=savedEnginePid_;
	MT4useVolume=useVolume_;
	MT4dt=dt_;
	MT4doDump=doDump_;

	//-- client Configurator ----------------------------------------------------------------
	strcpy_s(MT4clientXMLFile, MAX_PATH, clientXMLFile_);
	getFullPath(MT4clientXMLFile, clientffname);
	//-- load sCfg* for client
	safespawn(MT4clientCfg, newsname("MT4clientCfg"), defaultdbg, clientffname);
	//-- create client persistor, if needed
	bool saveClient;
	safecall(MT4clientCfg, setKey, "/Client");
	safecall(MT4clientCfg->currentKey, getParm, &saveClient, "saveClient");
	safespawn(MT4clientLog, newsname("ClientLogger"), defaultdbg, MT4clientCfg, "Persistor");
	//-----------------------------------------------------------------------------------------

	//-- random sessionId generation
	srand((unsigned int)time(NULL));
	MT4sessionId=MyRndInt(1, 1000000); info("MT4sessionId=%d", MT4sessionId);
}
void sRoot::MT4createEngine() {

	//-- check for possible duplicate pid in db (through client persistor), and change it
	safecall(this, getSafePid, MT4clientLog, &MT4clientPid);

	//-- spawn engine from savedEnginePid_ with pid
	safespawn(MT4engine, newsname("Engine"), defaultdbg, MT4clientLog, MT4clientPid, MT4enginePid);
	MT4sampleLen=MT4engine->shape->sampleLen;
	MT4predictionLen=MT4engine->shape->predictionLen;
	info("Environment initialized and Engine created for Account Number %d inferring from Engine pid %d using config from %s", MT4accountId, MT4enginePid, MT4clientXMLFile);
}
//--
extern "C" __declspec(dllexport) int _createEnv2(int accountId_, char* clientXMLFile_, int savedEnginePid_, bool useVolume_, int dt_, bool doDump_, char* oEnvS) {
	static sRoot* root;
	try {
		root=new sRoot(nullptr);
		sprintf_s(oEnvS, 64, "%p", root);
		root->setMT4env(GetCurrentProcessId(), accountId_, clientXMLFile_, savedEnginePid_, useVolume_, dt_, doDump_);
		//root->MT4createEngine();
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}
	return 0;
}
extern "C" __declspec(dllexport) int _getSeriesInfo(char* iEnvS, int* oSeriesCnt_, int* oSampleLen_, int* oPredictionLen_, char* oSymbolsCSL_, char* oTimeFramesCSL_, char* oFeaturesCSL_) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	env->dbg->out(DBG_MSG_INFO, __func__, 0, nullptr, "env=%p . Calling env->getSeriesInfo()...", env);
	try {
		env->getSeriesInfo(oSeriesCnt_, oSampleLen_, oPredictionLen_, oSymbolsCSL_, oTimeFramesCSL_, oFeaturesCSL_);
	}
	catch (std::exception exc) {
		return -1;
	}

	return 0;
}
//--
extern "C" __declspec(dllexport) int _createEnv(int accountId_, char* clientXMLFile_, int savedEnginePid_, bool useVolume_, int dt_, bool doDump_, char* oEnvS, int* oSampleLen_, int* oPredictionLen_) {

	static sRoot* root;
	try {
		root=new sRoot(nullptr);
		sprintf_s(oEnvS, 64, "%p", root);
		root->setMT4env(GetCurrentProcessId(), accountId_, clientXMLFile_, savedEnginePid_, useVolume_, dt_, doDump_);
		root->MT4createEngine();
		(*oSampleLen_)=root->MT4engine->shape->sampleLen;
		(*oPredictionLen_)=root->MT4engine->shape->predictionLen;
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}


	return 0;

}
extern "C" __declspec(dllexport) int _getForecast(char* iEnvS, long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, long iBaseBarT, double iBaseBarO, double iBaseBarH, double iBaseBarL, double iBaseBarC, double iBaseBarV, double* oForecastO, double* oForecastH, double* oForecastL, double* oForecastC, double* oForecastV) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	env->dbg->out(DBG_MSG_INFO, __func__, 0, nullptr, "env=%p . Calling env->getForecast()...", env);	
	try {
		env->getForecast(iBarT, iBarO, iBarH, iBarL, iBarC, iBarV, iBaseBarT, iBaseBarO, iBaseBarH, iBaseBarL, iBaseBarC, iBaseBarV, oForecastO, oForecastH, oForecastL, oForecastC, oForecastV);
	}
	catch (std::exception exc) {
		return -1;
	}

	return 0;
}
extern "C" __declspec(dllexport) int _saveTradeInfo(char* iEnvS, int iPositionTicket, long iPositionOpenTime, long iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	char iPositionOpenTimes[DATE_FORMAT_LEN];
	char iLastBarTs[DATE_FORMAT_LEN];
	try {
		//-- first, convert the 2 dates to string
		MT4time2str(iPositionOpenTime, DATE_FORMAT_LEN, iPositionOpenTimes);
		MT4time2str(iLastBarT, DATE_FORMAT_LEN, iLastBarTs);
		//-- then, make the call
		env->saveTradeInfo(iPositionTicket, iPositionOpenTimes, iLastBarTs, iLastBarO, iLastBarH, iLastBarL, iLastBarC, iLastBarV, iForecastO, iForecastH, iForecastL, iForecastC, iForecastV, iTradeScenario, iTradeResult);
	}
	catch (std::exception exc) {
		return -1;
	}
	return 0;
}
extern "C" __declspec(dllexport) int _destroyEnv(char* iEnvS) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	delete env;

	return 0;
}

//--
//#pragma pack(1)
extern "C" __declspec(dllexport) int kaz(int barsCnt, sMqlRates bar[]) {
	FILE* kazf;
	fopen_s(&kazf, "C:/temp/bars.csv", "w");
	fprintf(kazf, "=======\n");
	for (int b=0; b<barsCnt; b++) fprintf(kazf, "%f,%f,%f,%f \n", bar[b].open, bar[b].high, bar[b].low, bar[b].close/*, bar[b].tick_volume, bar[b].spread, bar[b].real_volume*/);
	fclose(kazf);
	return 1;
}

