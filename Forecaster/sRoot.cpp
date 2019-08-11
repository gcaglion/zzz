
#include "sRoot.h"
//#include <vld.h>

//-- constructor / destructor
sRoot::sRoot(NativeReportProgress* progressReporter) : sCfgObj(nullptr, newsname("RootObj"), defaultdbg, progressReporter, nullptr, "") {
	pid=GetCurrentProcessId();
	GUIreporter=progressReporter;
}
sRoot::~sRoot() {}

//-- core stuff
void sRoot::datasetPrepare(sTS* ts_, sEngine* eng_, sDS*** ds_, int dsBatchSize_, bool dsDoDump_, char* dsDumpPath_, bool loadEngine_){
	info("eng_->WNNdecompLevel=%d", eng_->WNNdecompLevel);
	info("engine->type=%d", eng_->type);
	(*ds_)=(sDS**)malloc((eng_->WNNdecompLevel+2)*sizeof(sDS*));
	safespawn((*ds_)[0], newsname("dataSet_Base"), defaultdbg, ts_, 0, eng_->sampleLen, eng_->targetLen, dsBatchSize_, dsDoDump_, dsDumpPath_);

	//-- update TRmin/max in (*ds_) , if inferring from a loaded engine
	if (loadEngine_) {
		(*ds_)[0]->TRmin=eng_->DStrMin;
		(*ds_)[0]->TRmax=eng_->DStrMax;
	} else {
		(*ds_)[0]->TRmin=ts_->TRmin;
		(*ds_)[0]->TRmax=ts_->TRmax;
		eng_->DStrMin=ts_->TRmin;
		eng_->DStrMax=ts_->TRmax;
	}
	 
	//-- timeseries wavelets, if engine is WNN
	if (eng_->type==ENGINE_WNN) {
		safecall(ts_, FFTcalc, eng_->WNNdecompLevel, eng_->WNNwaveletType);
		//-- build LFA dataset
		safespawn((*ds_)[1], newsname("dataSet_LFA"), defaultdbg, ts_, 1, eng_->sampleLen, eng_->targetLen, dsBatchSize_, dsDoDump_, dsDumpPath_);
		//-- build HFD datasets
		for (int l=0; l<eng_->WNNdecompLevel; l++) safespawn((*ds_)[2+l], newsname("dataSet_HFD%d", l), defaultdbg, ts_, 2+l, eng_->sampleLen, eng_->targetLen, dsBatchSize_, dsDoDump_, dsDumpPath_);
		//-- update TRmin/max
		if (loadEngine_) {
			for (int d=1; d<(eng_->WNNdecompLevel+1); d++) {
				(*ds_)[d]->TRmin=eng_->DSfftMin[d-1];
				(*ds_)[d]->TRmax=eng_->DSfftMax[d-1];
			}
		} else {
			for (int d=1; d<(eng_->WNNdecompLevel+1); d++) {
				(*ds_)[d]->TRmin=ts_->FFTmin[d-1];
				(*ds_)[d]->TRmax=ts_->FFTmax[d-1];
			}
			eng_->DSfftMin=ts_->FFTmin;
			eng_->DSfftMax=ts_->FFTmax;
		}
	}

	//-- scale DSs
	for(int d=0; d<(eng_->WNNdecompLevel+2); d++) safecall((*ds_)[d], scale, eng_->coreParms[d]->scaleMin[0], eng_->coreParms[d]->scaleMax[0]);

}
void sRoot::trainClient(int simulationId_, const char* clientXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr) {

	try {
		//-- 0. set full file name for each of the input files
		getFullPath(clientXMLfile_, clientffname);
		getFullPath(trainXMLfile_, trainffname);
		getFullPath(engineXMLfile_, engineffname);

		//-- 1. load separate sCfg* for client, trainDataset, Engine
		safespawn(clientCfg, newsname("clientCfg"), defaultdbg, clientffname);
		safespawn(trainCfg, newsname("trainCfg"), defaultdbg, trainffname);
		safespawn(engCfg, newsname("engineCfg"), defaultdbg, engineffname);

		//-- 5. create client persistor, if needed
		bool saveClient;
		safecall(clientCfg, setKey, "/Client");
		safecall(clientCfg->currentKey, getParm, &saveClient, "saveClient");
		safespawn(clientLog, newsname("ClientLogger"), defaultdbg, clientCfg, "Persistor");

		//-- check for possible duplicate pid in db (through client persistor), and change it
		safecall(this, getSafePid, clientLog, &pid);

		int _trainSampleLen, _trainTargetLen, _trainBatchSize; bool _doDump; char _dp[MAX_PATH]; char* _dumpPath=&_dp[0];
		safecall(trainCfg->currentKey, getParm, &_trainSampleLen, "SampleLen");
		safecall(trainCfg->currentKey, getParm, &_trainTargetLen, "TargetLen");
		safecall(trainCfg->currentKey, getParm, &_trainBatchSize, "BatchSize");
		safecall(trainCfg->currentKey, getParm, &_doDump, "Dump");
		safecall(trainCfg->currentKey, getParm, &_dumpPath, "DumpPath");

		sTS* trainTS; safespawn(trainTS, newsname("trainTimeSerie"), defaultdbg, trainCfg, "/TimeSerie");

		//-- 4. spawn engine the standard way
		safespawn(engine, newsname("TrainEngine"), defaultdbg, engCfg, "/Engine", _trainSampleLen, _trainTargetLen, trainTS->featuresCnt, pid);

		//-- prepare datasets
		sDS** trainDS; datasetPrepare(trainTS, engine, &trainDS, _trainBatchSize, _doDump, _dumpPath, false);

		//-- training cycle core
		timer->start();

		//-- do training (also populates datasets)
		safecall(engine, train, simulationId_, trainDS);

		//-- do infer on training data, without reloading engine
		safecall(engine, infer, simulationId_, trainDS, trainTS, 0);

		//-- Commit engine persistor data
		safecall(engine, commit);
		//-- stop timer, and save client info
		timer->stop(endtimeS);
		safecall(clientLog, saveClientInfo, pid, simulationId_, pid, "Root.Tester", timer->startTime, timer->elapsedTime, trainTS->timestamp[0], "", "", true, false, clientffname, "", trainffname, engineffname);
		//-- persist XML config parameters for Client,DataSet,Engine
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 0, clientCfg);
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 2, trainCfg);
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 3, engCfg);
		//-- Commit clientpersistor data
		safecall(clientLog, commit);

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

		int _inferSampleLen, _inferTargetLen, _inferBatchSize; bool _doDump;  char _dp[MAX_PATH]; char* _dumpPath=&_dp[0];
		safecall(inferCfg->currentKey, getParm, &_inferSampleLen, "SampleLen");
		safecall(inferCfg->currentKey, getParm, &_inferTargetLen, "TargetLen");
		safecall(inferCfg->currentKey, getParm, &_inferBatchSize, "BatchSize");
		safecall(inferCfg->currentKey, getParm, &_doDump, "Dump");
		safecall(inferCfg->currentKey, getParm, &_dumpPath, "DumpPath");

		sTS* inferTS; safespawn(inferTS, newsname("inferTimeSerie"), defaultdbg, inferCfg, "/TimeSerie");

		//-- spawn engine from savedEnginePid_ with pid
		safespawn(engine, newsname("Engine"), defaultdbg, clientLog, pid, savedEnginePid_);
		
		//-- prepare datasets
		sDS** inferDS; datasetPrepare(inferTS, engine, &inferDS, _inferBatchSize, _doDump, _dumpPath, true);

		//-- core infer cycle
		timer->start();

		//-- do inference (also populates datasets)
		safecall(engine, infer, simulationId_, inferDS, inferTS, savedEnginePid_);

		//-- commit engine persistor data
		safecall(engine, commit);
		//-- stop timer, and save client info
		timer->stop(endtimeS);
		safecall(clientLog, saveClientInfo, pid, simulationId_, savedEnginePid_, "Root.Tester", timer->startTime, timer->elapsedTime, "", inferTS->timestamp[0], "", false, true, clientffname, "", inferffname, "");
		//-- persist XML config parameters for Client,DataSet,Engine
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 0, clientCfg);
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 2, inferCfg);
		//-- Commit clientpersistor data
		safecall(clientLog, commit);

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
#include "../DataMgr/sTS.h"
#include "../DataMgr/sDS.h"
void sRoot::kaz() {

	sCfg* dsCfg; safespawn(dsCfg, newsname("dsCfg"), defaultdbg, "Config/ds.xml");
	sDS* ds1; safespawn(ds1, newsname("ds1"), defaultdbg, dsCfg, "/DataSet1");
	ds1->target2prediction(); ds1->dump();
	sDS* ds2; safespawn(ds2, newsname("ds2"), defaultdbg, dsCfg, "/DataSet2");
	ds2->target2prediction(); ds2->dump();
	sDS* ds[2]; ds[0]=ds1; ds[1]=ds2;
	sDS* ds12; safespawn(ds12, newsname("ds12"), defaultdbg, 2, ds);
	return;

	/*sCfg* clientCfg; safespawn(clientCfg, newsname("ClientCfg"), defaultdbg, "Config/10/Client.xml");
	sLogger* clientLog;
	bool saveClient;
	safecall(clientCfg, setKey, "/Client");
	*/
	//clientCfg->setKey("/Client");
	//safecall(clientCfg->currentKey, getParm, &saveClient, "saveClient");
	//safespawn(clientLog, newsname("ClientLogger"), defaultdbg, clientCfg, "Persistor");

	sCfg* tsCfg; safespawn(tsCfg, newsname("tsCfg"), defaultdbg, "Config/ts0.xml");
	sTS* tsActual; safespawn(tsActual, newsname("tsActual"), defaultdbg, tsCfg, "/");
	tsActual->FFTcalc(4, WNN_HAAR);


	return;
	tsActual->dump();
//	tsActual->untransform();
//	tsActual->dump();

	/*sCfg* dsCfg; safespawn(dsCfg, newsname("dsCfg"), defaultdbg, "Config/10/ds0b.xml");
	sDS* ds0; safespawn(ds0, newsname("ds0"), defaultdbg, dsCfg, "/");

	sCfg* dsiCfg; safespawn(dsiCfg, newsname("dsiCfg"), defaultdbg, "Config/10/ds0i.xml");
	sDS* ds0i; safespawn(ds0i, newsname("ds0i"), defaultdbg, dsiCfg, "/");
*/

	//	sDS* ds1; safespawn(ds1, newsname("ds1"), defaultdbg, dsCfg, "/");
//	ds0->target2prediction(); ds1->target2prediction();
//	ds0->dump(); 
//	ds1->dump();
//	sDS* ds[2]; ds[0]=ds0; ds[1]=ds1;
//	sDS* ds2; safespawn(ds2, newsname("ds2"), defaultdbg, 2, ds);
//	ds2->dump();
//	return;
//	ds0->scale(-1, 1);
//	ds0->dump();
//	ds0->unscale();
//	ds0->dump();

//	int seqLen=ds0->samplesCnt+ds0->sampleLen+ds0->targetLen-1;
//	numtype* trgSeq=(numtype*)malloc(seqLen*sizeof(numtype));
//	ds0->getSeq(TARGET, trgSeq);
//	dumpArrayH(seqLen, trgSeq, "C:/temp/dataDump/trgSeq.csv");
//	return;

	//--
/*	sCfg* engCfg; safespawn(engCfg, newsname("engCfg"), defaultdbg, "Config/10/Engine1.xml");
	int engpid=GetCurrentProcessId();
	sEngine* eng1; safespawn(eng1, newsname("SixCoresEngine"), defaultdbg, engCfg, "/Engine", ds0->sampleLen, ds0->targetLen, ds0->featuresCnt, engpid);
	eng1->train(1, ds0);
	eng1->saveMSE();
	eng1->saveCoreImages();
	eng1->saveCoreLoggers();
	eng1->saveInfo();
	eng1->infer(1, ds0, engpid, false);
	eng1->commit();
	delete eng1;

	safespawn(eng1, newsname("Engine"), defaultdbg, clientLog, pid, engpid);

	eng1->infer(1, ds0i, engpid, true);
	eng1->commit();
*/
	return;

/*	char ffname[MAX_PATH];
	char* BaseDataSetXMLFile="Config/30/DataSets/base.xml"; getFullPath(BaseDataSetXMLFile, ffname);
	int simulationId=99;
	int pid=GetCurrentProcessId();
	int tid=GetCurrentThreadId();
	srand(timeGetTime());

	//-- need client config to create a client persistor
	char* clientXMLfile="Config/master/99/Client.xml"; getFullPath(clientXMLfile, clientffname);
	safespawn(clientCfg, newsname("clientConfig"), erronlydbg, clientXMLfile);
	sLogger* clientPersistor= new sLogger(this, newsname("clientLogger"), erronlydbg, GUIreporter, clientCfg, "/Client/Persistor");

	sCfg* ds1Cfg; safespawn(ds1Cfg, newsname("ds1Config"), erronlydbg, "C:/users/gcaglion/dev/zzz/Config/play/ds1.xml");
	sDataSet* ds1t; safespawn(ds1t, newsname("ds1TRAIN"), defaultdbg, ds1Cfg, "/DataSet");
	sDataSet* ds1i; safespawn(ds1i, newsname("ds1INFER"), defaultdbg, ds1Cfg, "/DataSet");
	ds1t->build(ACTUAL, BASE);

	sTimeSerie* ts=ds1t->sourceTS[0];
	ts->dump(ACTUAL, TR);
*/
	/*for (int s=0; s<(ts->stepsCnt-3); s++) {
		for (int f=0; f<ts->featuresCnt; f++) {
			ts->val[PREDICTED][TR][s*ts->featuresCnt+f]=EMPTY_VALUE;
		}
	}
	for (int s=(ts->stepsCnt-3); s<ts->stepsCnt; s++) { 
		for (int f=0; f<ts->featuresCnt; f++) {
			ts->val[PREDICTED][TR][s*ts->featuresCnt+f]=ts->val[ACTUAL][TR][s*ts->featuresCnt+f]+1;
		}
	}
	
	ts->dump(ACTUAL, BASE);
	ts->dump(ACTUAL, TR);
	ts->dump(PREDICTED, TR);
	
//	ts->untrDELTA(PREDICTED);
	ts->dump(PREDICTED, BASE);
*/
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
		for (int f=0; f<ts1->featuresCnt; f++) {
			idx=s*ts1->featuresCnt+f;
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
void sRoot::getSeriesInfo(int* oSeriesCnt_, char* oSymbolsCSL_, char* oTimeFramesCSL_, char* oFeaturesCSL_, bool* oChartTrade_) {

	char tmpSymbol[XMLKEY_PARM_VAL_MAXLEN]; char tmpSymbolList[XMLKEY_PARM_VAL_MAXLEN]="";
	char tmpTF[XMLKEY_PARM_VAL_MAXLEN]; char tmpTFList[XMLKEY_PARM_VAL_MAXLEN]="";
	char* tmpC;

	int tmpFeaturesCnt; int tmpFeature[MAX_TS_FEATURES]; char tmpFeatureS[XMLKEY_PARM_VAL_MAXLEN]; char tmpFeaturesList[XMLKEY_PARM_VAL_MAXLEN]="";
	int* tmpFeatureP=&tmpFeature[0];

	safecall(MT4clientCfg->currentKey, getParm, oSeriesCnt_, "MetaTrader/ChartsCount");
	for (int s=0; s<(*oSeriesCnt_); s++) {
		tmpC=&tmpSymbol[0]; safecall(MT4clientCfg->currentKey, getParm, &tmpC, (newsname("MetaTrader/Chart%d/Symbol", s))->base);
		tmpC=&tmpTF[0]; safecall(MT4clientCfg->currentKey, getParm, &tmpC, (newsname("MetaTrader/Chart%d/TimeFrame", s))->base);
		//safecall(MT4clientCfg->currentKey, getParm, &oChartTrade_[s], (newsname("MetaTrader/Chart%d/Trade", s))->base);
		oChartTrade_[s]=true;
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

void sRoot::getForecast(int seriesCnt_, int dt_, int* featureMask_, long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, long* iBaseBarT, double* iBaseBarO, double* iBaseBarH, double* iBaseBarL, double* iBaseBarC, double* iBaseBarV, double* oForecastO, double* oForecastH, double* oForecastL, double* oForecastC, double* oForecastV) {

	//-- need to make a local copy of featureMask_, as it gets changed just to get selFcnt
	int* _featureMask=(int*)malloc(seriesCnt_*sizeof(int));
	memcpy_s(_featureMask, seriesCnt_*sizeof(int), featureMask_, seriesCnt_*sizeof(int));
	//-- _featureMask to selectedFeature[]
	int selFcntTot=0;
	int* selFcnt=(int*)malloc(seriesCnt_*sizeof(int));
	int** selF=(int**)malloc(seriesCnt_*sizeof(int*));
	for (int serie=0; serie<seriesCnt_; serie++) {
		selFcnt[serie]=0;
		selF[serie]=(int*)malloc(5*sizeof(int));
		if (_featureMask[serie]>=10000) { selF[serie][selFcnt[serie]]=FXOPEN; selFcnt[serie]++; _featureMask[serie]-=10000; }	//-- OPEN is selected
		if (_featureMask[serie]>=1000) { selF[serie][selFcnt[serie]]=FXHIGH; selFcnt[serie]++; _featureMask[serie]-=1000; }		//-- HIGH is selected
		if (_featureMask[serie]>=100) { selF[serie][selFcnt[serie]]=FXLOW; selFcnt[serie]++; _featureMask[serie]-=100; }		//-- LOW is selected
		if (_featureMask[serie]>=10) { selF[serie][selFcnt[serie]]=FXCLOSE; selFcnt[serie]++; _featureMask[serie]-=10; }		//-- CLOSE is selected
		if (_featureMask[serie]>=1) { selF[serie][selFcnt[serie]]=FXVOLUME; selFcnt[serie]++; _featureMask[serie]-=1; }			//-- VOLUME is selected
		info("serie[%d] featuresCnt=%d", serie, selFcnt[serie]);
		for (int sf=0; sf<selFcnt[serie]; sf++) info("serie[%d] feature [%d]=%d", serie, sf, selF[serie][sf]);
		selFcntTot+=selFcnt[serie];
	}

	numtype* oBar=(numtype*)malloc((MT4engine->sampleLen+MT4engine->targetLen)*selFcntTot*sizeof(numtype));	// flat, ordered by Bar,Feature
	long oBarTime;
	char** oBarTimeS=(char**)malloc((MT4engine->sampleLen+MT4engine->targetLen)*sizeof(char*)); for (int b=0; b<(MT4engine->sampleLen+MT4engine->targetLen); b++) oBarTimeS[b]=(char*)malloc(DATE_FORMAT_LEN);
	int fi=0;
	for (int b=0; b<MT4engine->sampleLen; b++) {
		for (int s=0; s<seriesCnt_; s++) {
			oBarTime=iBarT[s*MT4engine->sampleLen+b];
			MT4time2str(oBarTime, DATE_FORMAT_LEN, oBarTimeS[b]);
			for (int f=0; f<selFcnt[s]; f++) {
				if (selF[s][f]==FXOPEN)   oBar[fi]=(numtype)iBarO[s*MT4engine->sampleLen+b];
				if (selF[s][f]==FXHIGH)   oBar[fi]=(numtype)iBarH[s*MT4engine->sampleLen+b];
				if (selF[s][f]==FXLOW)    oBar[fi]=(numtype)iBarL[s*MT4engine->sampleLen+b];
				if (selF[s][f]==FXCLOSE)  oBar[fi]=(numtype)iBarC[s*MT4engine->sampleLen+b];
				if (selF[s][f]==FXVOLUME) oBar[fi]=(numtype)iBarV[s*MT4engine->sampleLen+b];
				fi++;
			}
		}
	}
	//--
	for (int b=0; b<MT4engine->targetLen; b++) {
		strcpy_s(oBarTimeS[MT4engine->sampleLen+b], DATE_FORMAT_LEN, "9999-99-99-99:99");
		for (int f=0; f<selFcntTot; f++) {
			oBar[fi]=EMPTY_VALUE;
			fi++;
		}
	}
	//--
	numtype* oBarB=(numtype*)malloc(selFcntTot*sizeof(numtype));
	char* oBarBTimeS=(char*)malloc(DATE_FORMAT_LEN);
	fi=0;
	for (int s=0; s<seriesCnt_; s++) {
		oBarTime=iBaseBarT[0];
		MT4time2str(oBarTime, DATE_FORMAT_LEN, oBarBTimeS);
		for (int f=0; f<selFcnt[s]; f++) {
			if (selF[s][f]==FXOPEN)   oBarB[fi]=(numtype)iBaseBarO[s];
			if (selF[s][f]==FXHIGH)   oBarB[fi]=(numtype)iBaseBarH[s];
			if (selF[s][f]==FXLOW)    oBarB[fi]=(numtype)iBaseBarL[s];
			if (selF[s][f]==FXCLOSE)  oBarB[fi]=(numtype)iBaseBarC[s];
			if (selF[s][f]==FXVOLUME) oBarB[fi]=(numtype)iBaseBarV[s];
			fi++;
		}
	}

	FILE* f;
	fopen_s(&f, "C:/temp/DataDump/oBar.csv", "w");
	for (int fi=0; fi<selFcntTot; fi++) fprintf(f, "%f,", oBarB[fi]);
	fprintf(f, ",%s\n", oBarBTimeS);
	//--
	int bi=0;
	for (int i=0; i<(MT4engine->sampleLen+MT4engine->targetLen)*selFcntTot; i++) {
		fprintf(f, "%f,", oBar[i]);
		if ((i+1)%selFcntTot==0) {
			fprintf(f, "%d,%s \n", i, oBarTimeS[bi]);
			bi++;
		}
	}
	fclose(f);
	//--
	sTS* mtTS; safespawn(mtTS, newsname("MTtimeSerie"), defaultdbg, MT4engine->sampleLen+MT4engine->targetLen, selFcntTot, dt_, oBarTimeS, oBar, oBarBTimeS, oBarB, MT4doDump);
	sDS** mtDS; safecall(this, datasetPrepare, mtTS, MT4engine, &mtDS, 1, MT4doDump,(char*)nullptr, true);
	//--
	safecall(MT4engine, infer, MT4accountId, mtDS, mtTS, MT4enginePid);

	for (int b=0; b<MT4engine->targetLen; b++) {
		for (int f=0; f<MT4engine->featuresCnt; f++) {
			info("forecast[%d]= %f", b*MT4engine->featuresCnt+f, MT4engine->forecast[b*MT4engine->featuresCnt+f]);
		}
	}
	fi=0;
	for (int b=0; b<MT4engine->targetLen; b++) {
		for (int s=0; s<seriesCnt_; s++) {
			for (int sf=0; sf<selFcnt[s]; sf++) {
				if (selF[s][sf]==FXOPEN) oForecastO[s*MT4engine->targetLen+b]=MT4engine->forecast[fi];
				if (selF[s][sf]==FXHIGH) oForecastH[s*MT4engine->targetLen+b]=MT4engine->forecast[fi];
				if (selF[s][sf]==FXLOW) oForecastL[s*MT4engine->targetLen+b]=MT4engine->forecast[fi];
				if (selF[s][sf]==FXCLOSE) oForecastC[s*MT4engine->targetLen+b]=MT4engine->forecast[fi];
				if (selF[s][sf]==FXVOLUME) oForecastV[s*MT4engine->targetLen+b]=MT4engine->forecast[fi];
				fi++;
			}
			info("OHLCV Forecast, serie %d , bar %d: %f|%f|%f|%f|%f", s, b, oForecastO[s*MT4engine->targetLen+b], oForecastH[s*MT4engine->targetLen+b], oForecastL[s*MT4engine->targetLen+b], oForecastC[s*MT4engine->targetLen+b], oForecastV[s*MT4engine->targetLen+b]);
		}
	}

	safecall(MT4engine, commit);

	//-- cleanup

	return;

}
void sRoot::saveTradeInfo(int iPositionTicket, char* iPositionOpenTime, char* iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult, int iTPhit, int iSLhit) {
	safecall(MT4clientLog, saveTradeInfo, MT4clientPid, MT4sessionId, MT4accountId, MT4enginePid, iPositionTicket, iPositionOpenTime, iLastBarT, iLastBarO, iLastBarH, iLastBarL, iLastBarC, iLastBarV, iForecastO, iForecastH, iForecastL, iForecastC, iForecastV, iTradeScenario, iTradeResult, iTPhit, iSLhit);
}
void sRoot::saveClientInfo(double iPositionOpenTime) {
	char accountStr[64]; sprintf_s(accountStr, 32, "MT5_account_%d", MT4accountId);
	info("MT4enginePid=%d", MT4enginePid);
	safecall(MT4clientLog, saveClientInfo, MT4clientPid, MT4sessionId, MT4enginePid, accountStr, iPositionOpenTime, 0, "", "","",false,true,"","","","");
}
void sRoot::setMT4env(int clientPid_, int accountId_, char* clientXMLFile_, int savedEnginePid_, int dt_, bool doDump_) {
	MT4clientPid=clientPid_;
	MT4accountId=accountId_;
	MT4enginePid=savedEnginePid_;
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
void sRoot::MT4createEngine(int* oSampleLen_, int* oPredictionLen_, int* oFeaturesCnt_) {

	//-- check for possible duplicate pid in db (through client persistor), and change it
	safecall(this, getSafePid, MT4clientLog, &MT4clientPid);

	//-- spawn engine from savedEnginePid_ with pid
	safespawn(MT4engine, newsname("Engine"), defaultdbg, MT4clientLog, MT4clientPid, MT4enginePid);
	
	(*oSampleLen_)=MT4engine->sampleLen;
	(*oPredictionLen_)=MT4engine->targetLen;
	(*oFeaturesCnt_)=MT4engine->featuresCnt;
	info("Engine spawned from DB. sampleLen=%d ; targetLen=%d ; featuresCnt=%d", MT4engine->sampleLen, MT4engine->targetLen, MT4engine->featuresCnt);
	info("Environment initialized and Engine created for Account Number %d inferring from Engine pid %d using config from %s", MT4accountId, MT4enginePid, MT4clientXMLFile);
}
void sRoot::MT4commit(){
	try {
		//-- commit engine persistor to keep saveRun
		MT4engine->commit();
		//-- commit client to keep saveTradeInfo
		MT4clientLog->commit();
	}
	catch (std::exception exc) {
	}
}
//--
extern "C" __declspec(dllexport) int _createEnv(int accountId_, char* clientXMLFile_, int savedEnginePid_, int dt_, bool doDump_, char* oEnvS, int* oSampleLen_, int* oPredictionLen_, int* oFeaturesCnt_) {
	static sRoot* root;
	try {
		root=new sRoot(nullptr);
		sprintf_s(oEnvS, 64, "%p", root);
		root->setMT4env(GetCurrentProcessId(), accountId_, clientXMLFile_, savedEnginePid_, dt_, doDump_);
		root->MT4createEngine(oSampleLen_, oPredictionLen_, oFeaturesCnt_);
	}
	catch (std::exception exc) {
		terminate(false, "Exception thrown by root. See stack.");
	}
	return 0;
}
extern "C" __declspec(dllexport) int _getSeriesInfo(char* iEnvS, int* oSeriesCnt_, char* oSymbolsCSL_, char* oTimeFramesCSL_, char* oFeaturesCSL_, bool* oChartTrade_) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	env->dbg->out(DBG_MSG_INFO, __func__, 0, nullptr, "env=%p . Calling env->getSeriesInfo()...", env);
	try {
		env->getSeriesInfo(oSeriesCnt_, oSymbolsCSL_, oTimeFramesCSL_, oFeaturesCSL_, oChartTrade_);
	}
	catch (std::exception exc) {
		return -1;
	}

	return 0;
}
//--
extern "C" __declspec(dllexport) int _getForecast(char* iEnvS, int seriesCnt_, int dt_, int* featureMask_, long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, long* iBaseBarT, double* iBaseBarO, double* iBaseBarH, double* iBaseBarL, double* iBaseBarC, double* iBaseBarV, double* oForecastO, double* oForecastH, double* oForecastL, double* oForecastC, double* oForecastV) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	env->dbg->out(DBG_MSG_INFO, __func__, 0, nullptr, "env=%p . Calling env->getForecast()...", env);	
	try {
		env->getForecast(seriesCnt_, dt_, featureMask_, iBarT, iBarO, iBarH, iBarL, iBarC, iBarV, iBaseBarT, iBaseBarO, iBaseBarH, iBaseBarL, iBaseBarC, iBaseBarV, oForecastO, oForecastH, oForecastL, oForecastC, oForecastV);
	}
	catch (std::exception exc) {
		return -1;
	}

	return 0;
}
extern "C" __declspec(dllexport) int _saveTradeInfo(char* iEnvS, int iPositionTicket, long iPositionOpenTime, long iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult, int iTPhit, int iSLhit) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	char iPositionOpenTimes[DATE_FORMAT_LEN];
	char iLastBarTs[DATE_FORMAT_LEN];
	try {
		//-- first, convert the 2 dates to string
		MT4time2str(iPositionOpenTime, DATE_FORMAT_LEN, iPositionOpenTimes);
		MT4time2str(iLastBarT, DATE_FORMAT_LEN, iLastBarTs);
		//-- then, make the call
		env->saveTradeInfo(iPositionTicket, iPositionOpenTimes, iLastBarTs, iLastBarO, iLastBarH, iLastBarL, iLastBarC, iLastBarV, iForecastO, iForecastH, iForecastL, iForecastC, iForecastV, iTradeScenario, iTradeResult, iTPhit, iSLhit);
	}
	catch (std::exception exc) {
		return -1;
	}
	return 0;
}
extern "C" __declspec(dllexport) int _saveClientInfo(char* iEnvS, long iTradeStartTime) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	char iPositionOpenTimes[DATE_FORMAT_LEN];
	try {
		//-- first, convert the 2 dates to string
		MT4time2str(iTradeStartTime, DATE_FORMAT_LEN, iPositionOpenTimes);
		//-- then, make the call
		env->saveClientInfo(iTradeStartTime);
	}
	catch (std::exception exc) {
		return -1;
	}
	return 0;
}
extern "C" __declspec(dllexport) void _commit(char* iEnvS) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	env->MT4commit();
	
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

