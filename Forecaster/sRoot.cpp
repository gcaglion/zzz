
#include "sRoot.h"
//#include <vld.h>

//-- constructor / destructor
sRoot::sRoot(NativeReportProgress* progressReporter) : sCfgObj(nullptr, newsname("RootObj"), defaultdbg, progressReporter, nullptr, "") {
	pid=GetCurrentProcessId();
	GUIreporter=progressReporter;
}
sRoot::~sRoot() {}

//-- core stuff
void sRoot::trainClient(int simulationId_, const char* clientXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr, int overridesCnt_, char** overridePname_, char** overridePval_) {
	
	try {
		//-- 0. set full file name for each of the input files
		getFullPath(clientXMLfile_, clientffname);
		getFullPath(trainXMLfile_, trainffname);
		getFullPath(engineXMLfile_, engineffname);

		sCfg* clientCfg; safespawn(clientCfg, newsname("clientCfg"), defaultdbg, clientffname);
		sCfg* trainCfg; safespawn(trainCfg, newsname("trainCfg"), defaultdbg, trainffname);
		sCfg* engCfg; safespawn(engCfg, newsname("engCfg"), defaultdbg, engineffname);

		//-- 1.1. override parameters from command line
		for (int o=0; o<overridesCnt_; o++) {
			replace(overridePname_[o], '.', '/');
			if (overridePname_[o][0]=='c'&&overridePname_[o][1]=='l'&&overridePname_[o][2]=='i') {}
			if (overridePname_[o][0]=='d'&&overridePname_[o][1]=='s') {
				trainCfg->rootKey->setParm(&overridePname_[o][3], overridePval_[o]);
			}
			if (overridePname_[o][0]=='e'&&overridePname_[o][1]=='n'&&overridePname_[o][2]=='g') {
				engCfg->rootKey->setParm(&overridePname_[o][4], overridePval_[o]);
			}
		}

		sLogger* clientLogger; safespawn(clientLogger, newsname("clientLogger"), defaultdbg, clientCfg, "/Client/Persistor");
		safecall(this, getSafePid, clientLogger, &pid);

		safecall(trainCfg->currentKey, getParm, &_sampleLen, "SampleLen");
		safecall(trainCfg->currentKey, getParm, &_targetLen, "TargetLen");
		safecall(trainCfg->currentKey, getParm, &_batchSize, "BatchSize");
		sTS2* trainTS; safespawn(trainTS, newsname("trainTS"), defaultdbg, trainCfg, "/TimeSerie");

		sEngine* eng; safespawn(eng, newsname("Engine"), defaultdbg, engCfg, "/");

		safecall(eng, train, simulationId_, trainTS, _sampleLen, _targetLen, _batchSize);

		//-- check if break with no save was requested
		if (eng->core[0]->procArgs->quitAfterBreak) return;

		timer->stop(endtimeS);
		safecall(clientLogger, saveClientInfo, pid, 0, simulationId_, pid, "Root.Tester", timer->startTime, timer->elapsedTime, trainTS->timestamp[0], "", "", true, false, "", "", "", "");

		eng->commit();
		clientLogger->commit();

	}
	catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}

}
void sRoot::inferClient(int simulationId_, const char* clientXMLfile_, const char* inferXMLfile_, int savedEnginePid_, NativeReportProgress* progressPtr, int overridesCnt_, char** overridePname_, char** overridePval_) {
	//-- 0. set full file name for each of the input files
	getFullPath(clientXMLfile_, clientffname);
	getFullPath(inferXMLfile_, inferffname);

	//-- 1. load separate sCfg* for client, inferDataset, Engine
	sCfg* clientCfg; safespawn(clientCfg, newsname("clientCfg"), defaultdbg, clientffname);
	sCfg* inferCfg; safespawn(inferCfg, newsname("inferCfg"), defaultdbg, inferffname);

	//-- 1.1. override parameters from command line
	for (int o=0; o<overridesCnt_; o++) {
		replace(overridePname_[o], '.', '/');
		if (overridePname_[o][0]=='c'&&overridePname_[o][1]=='l'&&overridePname_[o][2]=='i') {}
		if (overridePname_[o][0]=='d'&&overridePname_[o][1]=='s') {
			inferCfg->rootKey->setParm(&overridePname_[o][3], overridePval_[o]);
		}
	}

	sLogger* clientLogger; safespawn(clientLogger, newsname("clientLogger"), defaultdbg, clientCfg, "/Client/Persistor");
	safecall(this, getSafePid, clientLogger, &pid);

	safecall(inferCfg->currentKey, getParm, &_sampleLen, "SampleLen");
	safecall(inferCfg->currentKey, getParm, &_targetLen, "TargetLen");
	safecall(inferCfg->currentKey, getParm, &_batchSize, "BatchSize");
	sTS2* inferTS; safespawn(inferTS, newsname("inferTS"), defaultdbg, inferCfg, "/TimeSerie");

	sEngine* eng; safespawn(eng, newsname("Engine"), defaultdbg, clientLogger, pid, savedEnginePid_);

	safecall(eng, infer, simulationId_, 0, inferTS, savedEnginePid_);

	timer->stop(endtimeS);
	clientLogger->saveClientInfo(pid, 0, simulationId_, savedEnginePid_, "Root.Tester", timer->startTime, timer->elapsedTime, "", inferTS->timestamp[0], "", false, true, "", "", "", "");

	eng->commit();
	clientLogger->commit();
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
#include "../DataMgr/sTS2.h"
#include "../EngineMgr/sCoreLayout.h"
#include "../EngineMgr/sNNparms.h"
#include "../EngineMgr/sNN2.h"

void createBars(int n, long** iBarT, double** iBarO, double** iBarH, double** iBarL, double** iBarC, double** iBarV, long** iBaseBarT, double** iBaseBarO, double** iBaseBarH, double** iBaseBarL, double** iBaseBarC, double** iBaseBarV) {
	srand(timeGetTime());
	(*iBarT)=(long*)malloc(n*sizeof(long));
	(*iBarO)=(double*)malloc(n*sizeof(double));
	(*iBarH)=(double*)malloc(n*sizeof(double));
	(*iBarL)=(double*)malloc(n*sizeof(double));
	(*iBarC)=(double*)malloc(n*sizeof(double));
	(*iBarV)=(double*)malloc(n*sizeof(double));
	(*iBaseBarT)=(long*)malloc(n*sizeof(long));
	(*iBaseBarO)=(double*)malloc(n*sizeof(double));
	(*iBaseBarH)=(double*)malloc(n*sizeof(double));
	(*iBaseBarL)=(double*)malloc(n*sizeof(double));
	(*iBaseBarC)=(double*)malloc(n*sizeof(double));
	(*iBaseBarV)=(double*)malloc(n*sizeof(double));
	for (int i=0; i<n; i++) (*iBarT)[i]=rand();
	VinitRnd(n, -1, 1, (*iBarO));
	VinitRnd(n, -1, 1, (*iBarH));
	VinitRnd(n, -1, 1, (*iBarL));
	VinitRnd(n, -1, 1, (*iBarC));
	VinitRnd(n, -1, 1, (*iBarV));
	for (int i=0; i<n; i++) (*iBaseBarT)[i]=rand();
	VinitRnd(n, -1, 1, (*iBaseBarO));
	VinitRnd(n, -1, 1, (*iBaseBarH));
	VinitRnd(n, -1, 1, (*iBaseBarL));
	VinitRnd(n, -1, 1, (*iBaseBarC));
	VinitRnd(n, -1, 1, (*iBaseBarV));
}

void sRoot::kaz() {

}
/*void sRoot::kazEnc() {
	sAlgebra* Alg = new sAlgebra(nullptr, newsname("Alg"), defaultdbg, nullptr);

	sCfg* tsCfg = new sCfg(nullptr, newsname("tsCfg"), defaultdbg, nullptr, "Config/trainDS.xml");
	sTS2* ts1 = new sTS2(nullptr, newsname("ts1"), defaultdbg, nullptr, tsCfg, "/TimeSerie");

	int sampleLen=25;
	int samplesCnt=ts1->stepsCnt-sampleLen;
	numtype* sample_h=(numtype*)malloc(samplesCnt*sampleLen*ts1->*sizeof(numtype));
	int tsidxS, dsidxS=0;
	for (int s=0; s<samplesCnt; s++) {
		for (int b=0; b<sampleLen; b++) {
			for (int f=0; f<ts1->featuresCnt; f++) {
				tsidxS=(s+b)*ts1->featuresCnt+f;
				sample_h[dsidxS]=ts1->valTR[tsidxS];
				dsidxS++;
			}
		}
	}
	numtype* sample_d; Alg->myMalloc(&sample_d, samplesCnt*sampleLen*ts1->featuresCnt);
	Alg->h2d(sample_d, sample_h, samplesCnt*sampleLen*ts1->featuresCnt*sizeof(numtype), true);

	const int levelsCnt=4;
	float levelRatio[levelsCnt-1]={ 0.5,1,0.5 };
	numtype learningRate=0.01f;
	numtype learningMomentum= 0.8f;
	int activationFunction[levelsCnt*2-1]={ NN_ACTIVATION_TANH, NN_ACTIVATION_TANH, NN_ACTIVATION_TANH, NN_ACTIVATION_TANH, NN_ACTIVATION_TANH, NN_ACTIVATION_TANH, NN_ACTIVATION_TANH };
	sEncoder* enc= new sEncoder(nullptr, newsname("encoder1"), defaultdbg, nullptr, sampleLen*ts1->featuresCnt, levelsCnt, levelRatio, activationFunction, learningRate, learningMomentum);

	int trainBatchSize=15029;
	int maxEpochs=50;
	enc->train(samplesCnt, sampleLen, ts1->featuresCnt, sample_d, trainBatchSize, maxEpochs);

	free(sample_h);
	Alg->myFree(sample_d);
}
*/
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

void sRoot::getForecast(int seqId_, int seriesCnt_, int dt_, int* featureMask_, \
						long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV,	\
						long* iBaseBarT, double* iBaseBarO, double* iBaseBarH, double* iBaseBarL, double* iBaseBarC, double* iBaseBarV,	\
						double* oForecastO, double* oForecastH, double* oForecastL, double* oForecastC, double* oForecastV \
						) {
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
		//info("serie[%d] featuresCnt=%d", serie, selFcnt[serie]);
		//for (int sf=0; sf<selFcnt[serie]; sf++) info("serie[%d] feature [%d]=%d", serie, sf, selF[serie][sf]);
		selFcntTot+=selFcnt[serie];
	}

	int sampleBarsCnt=MT4engine->sampleLen+MT4engine->batchSize-1;
	int targetBarsCnt=MT4engine->targetLen;
	numtype* oBar=(numtype*)malloc((sampleBarsCnt+targetBarsCnt)*selFcntTot*sizeof(numtype));	// flat, ordered by Bar,Feature
	long oBarTime;
	char** oBarTimeS=(char**)malloc((sampleBarsCnt+targetBarsCnt)*sizeof(char*)); for (int b=0; b<(sampleBarsCnt+targetBarsCnt); b++) oBarTimeS[b]=(char*)malloc(DATE_FORMAT_LEN);
	int fi=0;

	for (int b=0; b<(sampleBarsCnt); b++) {
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
	
	for (int b=0; b<targetBarsCnt; b++) {
		sprintf_s(oBarTimeS[sampleBarsCnt+b], DATE_FORMAT_LEN, "9999-99-99-99:%02d", b);
		for (int f=0; f<selFcntTot; f++) {
			oBar[fi]=oBar[(sampleBarsCnt-targetBarsCnt)*selFcntTot+b*selFcntTot+f];//EMPTY_VALUE;
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

	//--
	sTS2* mtTS; safespawn(mtTS, newsname("MTtimeSerie"), defaultdbg, \
		sampleBarsCnt+targetBarsCnt, oBarTimeS, oBarBTimeS, dt_, MT4doDump, \
		seriesCnt_, selFcnt, MT4engine->WTtype[0], MT4engine->WTlevel[0], oBar, oBarB, \
		MT4engine->dataSourcesCnt[1], MT4engine->featuresCnt[1], MT4engine->WTtype[1], MT4engine->WTlevel[1], (numtype*)nullptr, (numtype*)nullptr
	);
	safecall(MT4engine, infer, MT4accountId, seqId_, mtTS, MT4enginePid);

	fi=0;
	for (int b=0; b<MT4engine->targetLen; b++) {
		for (int s=0; s<seriesCnt_; s++) {
			for (int sf=0; sf<selFcnt[s]; sf++) {
				
				MT4engine->forecast[fi]=mtTS->prd[sampleBarsCnt+b][0][s][sf][0];

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

}
void sRoot::saveTradeInfo(int iPositionTicket, char* iPositionOpenTime, char* iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iLastForecastO, double iLastForecastH, double iLastForecastL, double iLastForecastC, double iLastForecastV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult, int iTPhit, int iSLhit) {
	safecall(MT4clientLog, saveTradeInfo, MT4clientPid, MT4sessionId, MT4accountId, MT4enginePid, iPositionTicket, iPositionOpenTime, iLastBarT, iLastBarO, iLastBarH, iLastBarL, iLastBarC, iLastBarV, iLastForecastO, iLastForecastH, iLastForecastL, iLastForecastC, iLastForecastV, iForecastO, iForecastH, iForecastL, iForecastC, iForecastV, iTradeScenario, iTradeResult, iTPhit, iSLhit);
}
void sRoot::saveClientInfo(int sequenceId, double iPositionOpenTime) {
	char accountStr[64]; sprintf_s(accountStr, 32, "MT5_account_%d", MT4accountId);
	info("MT4enginePid=%d", MT4enginePid);
	safecall(MT4clientLog, saveClientInfo, MT4clientPid, sequenceId, MT4sessionId, MT4enginePid, accountStr, iPositionOpenTime, 0, "", "","",false,true,"","","","");
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
void sRoot::MT4createEngine(int* oSampleLen_, int* oPredictionLen_, int* oFeaturesCnt_, int* oBatchSize) {

	//-- check for possible duplicate pid in db (through client persistor), and change it
	safecall(this, getSafePid, MT4clientLog, &MT4clientPid);

	//-- spawn engine from savedEnginePid_ with pid
	safespawn(MT4engine, newsname("Engine"), defaultdbg, MT4clientLog, MT4clientPid, MT4enginePid);

	(*oSampleLen_)=MT4engine->sampleLen;
	(*oPredictionLen_)=MT4engine->targetLen;
	(*oFeaturesCnt_)=MT4engine->inputCnt/MT4engine->sampleLen/(MT4engine->WTlevel[0]+2);
	(*oBatchSize)=MT4engine->batchSize;
	info("Engine spawned from DB. sampleLen=%d ; targetLen=%d ; featuresCnt=%d ; batchSize=%d", (*oSampleLen_), (*oPredictionLen_), (*oFeaturesCnt_), (*oBatchSize));
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
extern "C" __declspec(dllexport) int _createEnv(int accountId_, char* clientXMLFile_, int savedEnginePid_, int dt_, bool doDump_, char* oEnvS, int* oSampleLen_, int* oPredictionLen_, int* oFeaturesCnt_, int* oBatchSize_) {
	static sRoot* root;
	try {
		root=new sRoot(nullptr);
		sprintf_s(oEnvS, 64, "%p", root);
		root->setMT4env(GetCurrentProcessId(), accountId_, clientXMLFile_, savedEnginePid_, dt_, doDump_);
		root->MT4createEngine(oSampleLen_, oPredictionLen_, oFeaturesCnt_, oBatchSize_);
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
extern "C" __declspec(dllexport) int _getForecast(char* iEnvS, int seqId_, int seriesCnt_, int dt_, int* featureMask_, long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, long* iBaseBarT, double* iBaseBarO, double* iBaseBarH, double* iBaseBarL, double* iBaseBarC, double* iBaseBarV, double* oForecastO, double* oForecastH, double* oForecastL, double* oForecastC, double* oForecastV) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	env->dbg->out(DBG_MSG_INFO, __func__, 0, nullptr, "env=%p . Calling env->getForecast()...", env);	
	try {
		env->getForecast(seqId_, seriesCnt_, dt_, featureMask_, iBarT, iBarO, iBarH, iBarL, iBarC, iBarV, iBaseBarT, iBaseBarO, iBaseBarH, iBaseBarL, iBaseBarC, iBaseBarV, oForecastO, oForecastH, oForecastL, oForecastC, oForecastV);
	}
	catch (std::exception exc) {
		return -1;
	}

	return 0;
}
extern "C" __declspec(dllexport) int _saveTradeInfo(char* iEnvS, int iPositionTicket, long iPositionOpenTime, long iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iLastForecastO, double iLastForecastH, double iLastForecastL, double iLastForecastC, double iLastForecastV, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult, int iTPhit, int iSLhit) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	char iPositionOpenTimes[DATE_FORMAT_LEN];
	char iLastBarTs[DATE_FORMAT_LEN];
	try {
		//-- first, convert the 2 dates to string
		MT4time2str(iPositionOpenTime, DATE_FORMAT_LEN, iPositionOpenTimes);
		MT4time2str(iLastBarT, DATE_FORMAT_LEN, iLastBarTs);
		//-- then, make the call
		env->saveTradeInfo(iPositionTicket, iPositionOpenTimes, iLastBarTs, iLastBarO, iLastBarH, iLastBarL, iLastBarC, iLastBarV, iLastForecastO, iLastForecastH, iLastForecastL, iLastForecastC, iLastForecastV, iForecastO, iForecastH, iForecastL, iForecastC, iForecastV, iTradeScenario, iTradeResult, iTPhit, iSLhit);
	}
	catch (std::exception exc) {
		return -1;
	}
	return 0;
}
extern "C" __declspec(dllexport) int _saveClientInfo(char* iEnvS, int sequenceId, long iTradeStartTime) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	char iPositionOpenTimes[DATE_FORMAT_LEN];
	try {
		//-- first, convert the 2 dates to string
		MT4time2str(iTradeStartTime, DATE_FORMAT_LEN, iPositionOpenTimes);
		//-- then, make the call
		env->saveClientInfo(sequenceId, iTradeStartTime);
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
void sRoot::getActualFuture(char* iSymbol_, char* iTF_, char* iDate0_, char* oDate1_, double* oBarO, double* oBarH, double* oBarL, double* oBarC, double* oBarV) {
	info("%s() CheckPoint %d ; iSymbol=%s , iTF_=%s", __func__, 1, iSymbol_, iTF_);
	sOraData* fxdbconn; safespawn(fxdbconn, newsname("futureFXDBconn"), defaultdbg, "History", "HistoryPwd", "Algo");
	info("%s() CheckPoint %d ; iSymbol=%s , iTF_=%s", __func__, 2, iSymbol_, iTF_);
	sFXDataSource* fxdb; safespawn(fxdb, newsname("futureFXDB"), dbg, fxdbconn, iSymbol_, iTF_, false);
	info("%s() CheckPoint %d", __func__, 3);
	safecall(fxdb, loadFuture, iSymbol_, iTF_, iDate0_, oDate1_, oBarO, oBarH, oBarL, oBarC, oBarV);
	info("%s() CheckPoint %d ; oDate1=%s ; oBarH=%f ; oBarL=%f", __func__, 4, oDate1_, (*oBarH), (*oBarL));
	//-- introduce some random error
	srand((unsigned int)time(NULL));
	numtype errMax=0.0000;
	numtype kaz=(numtype)rand();
	numtype errH=(numtype)rand()/RAND_MAX*2*errMax-errMax;
	numtype errL=(numtype)rand()/RAND_MAX*2*errMax-errMax;
	numtype errC=(numtype)rand()/RAND_MAX*2*errMax-errMax;
	oBarH[0]+=errH; oBarL[0]+=errL; oBarC[0]+=errC;
	delete fxdb;
	delete fxdbconn;
}
extern "C" __declspec(dllexport) int _getActualFuture(char* iEnvS, char* iSymbol_, char* iTF_, char* iDate0_, char* oDate1_, double* oBarO, double* oBarH, double* oBarL, double* oBarC, double* oBarV) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	env->dbg->out(DBG_MSG_INFO, __func__, 0, nullptr, "env=%p . Calling env->getActualFuture()...", env);
	try {
		env->getActualFuture(iSymbol_, iTF_, iDate0_, oDate1_, oBarO, oBarH, oBarL, oBarC, oBarV);
	}
	catch (std::exception exc) {
		return -1;
	}

	return 0;
}
