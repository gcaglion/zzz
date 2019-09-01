
#include "sRoot.h"
//#include <vld.h>

//-- constructor / destructor
sRoot::sRoot(NativeReportProgress* progressReporter) : sCfgObj(nullptr, newsname("RootObj"), defaultdbg, progressReporter, nullptr, "") {
	pid=GetCurrentProcessId();
	GUIreporter=progressReporter;
}
sRoot::~sRoot() {}

//-- core stuff
void sRoot::datasetPrepare(sTS* ts_, sEngine* eng_, sDS*** ds_, int dsSampleLen_, int dsTargetLen_, int dsBatchSize_, bool dsDoDump_, char* dsDumpPath_, bool loadEngine_){
	info("eng_->WNNdecompLevel=%d", eng_->WNNdecompLevel);
	info("engine->type=%d", eng_->type);
	(*ds_)=(sDS**)malloc((eng_->WNNdecompLevel+2)*sizeof(sDS*));
	safespawn((*ds_)[0], newsname("dataSet_Base"), defaultdbg, ts_, 0, dsSampleLen_, dsTargetLen_, dsBatchSize_, dsDoDump_, dsDumpPath_);

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
void sRoot::trainClient(int simulationId_, const char* clientXMLfile_, const char* trainXMLfile_, const char* engineXMLfile_, NativeReportProgress* progressPtr, int overridesCnt_, char** overridePname_, char** overridePval_) {

	try {
		//-- 0. set full file name for each of the input files
		getFullPath(clientXMLfile_, clientffname);
		getFullPath(trainXMLfile_, trainffname);
		getFullPath(engineXMLfile_, engineffname);

		//-- 1. load separate sCfg* for client, trainDataset, Engine
		safespawn(clientCfg, newsname("clientCfg"), defaultdbg, clientffname, 0, overridesCnt_, overridePname_, overridePval_);
		safespawn(trainCfg, newsname("trainCfg"), defaultdbg, trainffname, 0, overridesCnt_, overridePname_, overridePval_);
		safespawn(engCfg, newsname("engineCfg"), defaultdbg, engineffname, 0, overridesCnt_, overridePname_, overridePval_);

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
		safespawn(engine, newsname("TrainEngine"), defaultdbg, engCfg, "/", _trainSampleLen, _trainTargetLen, trainTS->featuresCnt, _trainBatchSize, pid);

		//-- prepare datasets
		sDS** trainDS; datasetPrepare(trainTS, engine, &trainDS, _trainSampleLen, _trainTargetLen, _trainBatchSize, _doDump, _dumpPath, false);

		//-- training cycle core
		timer->start();

		//-- do training (also populates datasets)
		safecall(engine, train, simulationId_, trainDS);

		//-- check if break with no save was requested
		if (engine->core[0]->procArgs->quitAfterBreak) return;

		//-- do infer on training data, without reloading engine
		safecall(engine, infer, simulationId_, trainDS, trainTS, 0);

		//-- Commit engine persistor data
		safecall(engine, commit);
		//-- stop timer, and save client info
		timer->stop(endtimeS);
		safecall(clientLog, saveClientInfo, pid, simulationId_, pid, "Root.Tester", timer->startTime, timer->elapsedTime, trainTS->timestamp[0], "", "", true, false, clientffname, "", trainffname, engineffname);
		//-- persist XML config parameters for Client,DataSet,Engine
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 0, clientCfg);
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 1, trainCfg);
		safecall(clientLog, saveXMLconfig, simulationId_, pid, 0, 3, engCfg);
		//-- Commit clientpersistor data
		safecall(clientLog, commit);

	}
	catch (std::exception exc) {
		fail("Exception=%s", exc.what());
	}


}
void sRoot::inferClient(int simulationId_, const char* clientXMLfile_, const char* inferXMLfile_, int savedEnginePid_, NativeReportProgress* progressPtr, int overridesCnt_, char** overridePname_, char** overridePval_) {

	try {
		//-- 0. set full file name for each of the input files
		getFullPath(clientXMLfile_, clientffname);
		getFullPath(inferXMLfile_, inferffname);

		//-- 1. load separate sCfg* for client, inferDataset, Engine
		safespawn(clientCfg, newsname("clientCfg"), defaultdbg, clientffname);
		safespawn(inferCfg, newsname("inferCfg"), defaultdbg, inferffname);

		//-- 1.1. override parameters from command line
		for (int o=0; o<overridesCnt_; o++) {
			replace(overridePname_[o], '.', '/');
			if (overridePname_[o][0]=='c'&&overridePname_[o][1]=='l'&&overridePname_[o][2]=='i') {}
			if (overridePname_[o][0]=='d'&&overridePname_[o][1]=='s') {
				inferCfg->rootKey->setParm(&overridePname_[o][3], overridePval_[o]);
			}
		}

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
		sDS** inferDS; datasetPrepare(inferTS, engine, &inferDS, _inferSampleLen, _inferTargetLen, _inferBatchSize, _doDump, _dumpPath, true);

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
	sAlgebra* Alg; safespawn(Alg, newsname("Alg"), defaultdbg);
	numtype* v1; numtype* v2; numtype vdotv;
	Alg->myMalloc(&v1, 5); Alg->myMalloc(&v2, 5);
	Alg->VdotV(5, v1, v2, &vdotv);
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

	numtype* oBar=(numtype*)malloc((MT4engine->sampleLen+MT4engine->targetLen+MT4engine->batchSize-1)*selFcntTot*sizeof(numtype));	// flat, ordered by Bar,Feature
	long oBarTime;
	char** oBarTimeS=(char**)malloc((MT4engine->sampleLen+MT4engine->targetLen+MT4engine->batchSize-1)*sizeof(char*)); for (int b=0; b<(MT4engine->sampleLen+MT4engine->targetLen+MT4engine->batchSize-1); b++) oBarTimeS[b]=(char*)malloc(DATE_FORMAT_LEN);
	int fi=0;
	for (int b=0; b<(MT4engine->sampleLen+MT4engine->batchSize-1); b++) {
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
		strcpy_s(oBarTimeS[MT4engine->sampleLen+b+MT4engine->batchSize-1], DATE_FORMAT_LEN, "9999-99-99-99:99");
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

	/*FILE* f;
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
	fclose(f);*/
	//--
	sTS* mtTS; safespawn(mtTS, newsname("MTtimeSerie"), defaultdbg, MT4engine->sampleLen+MT4engine->targetLen+MT4engine->batchSize-1, selFcntTot, dt_, oBarTimeS, oBar, oBarBTimeS, oBarB, MT4doDump);
	sDS** mtDS; safecall(this, datasetPrepare, mtTS, MT4engine, &mtDS, MT4engine->sampleLen, MT4engine->targetLen, MT4engine->batchSize, MT4doDump,(char*)nullptr, true);
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
void sRoot::MT4createEngine(int* oSampleLen_, int* oPredictionLen_, int* oFeaturesCnt_, int* oBatchSize) {

	//-- check for possible duplicate pid in db (through client persistor), and change it
	safecall(this, getSafePid, MT4clientLog, &MT4clientPid);

	//-- spawn engine from savedEnginePid_ with pid
	safespawn(MT4engine, newsname("Engine"), defaultdbg, MT4clientLog, MT4clientPid, MT4enginePid);

	(*oSampleLen_)=MT4engine->sampleLen;
	(*oPredictionLen_)=MT4engine->targetLen;
	(*oFeaturesCnt_)=MT4engine->featuresCnt;
	(*oBatchSize)=MT4engine->batchSize;
	info("Engine spawned from DB. sampleLen=%d ; targetLen=%d ; featuresCnt=%d ; batchSize=%d", MT4engine->sampleLen, MT4engine->targetLen, MT4engine->featuresCnt, MT4engine->batchSize);
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

