
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

		sTS2* trainTS; safespawn(trainTS, newsname("trainTS"), defaultdbg, trainCfg, "/TimeSerie");

		sEngine* eng; safespawn(eng, newsname("Engine"), defaultdbg, engCfg, "/", pid);

		safecall(eng, train, simulationId_, trainTS);

		//-- check if break with no save was requested
		if (eng->core[0]->procArgs->quitAfterBreak) return;

		timer->stop(endtimeS);
		safecall(clientLogger, saveClientInfo, pid, 0, simulationId_, pid, "Root.Tester", timer->startTime, timer->elapsedTime, trainTS->timestamp[0][0], "", "", true, false, "", "", "", "");

		//-- persist XML config parameters for Client,DataSet,Engine
		safecall(clientLogger, saveXMLconfig, simulationId_, pid, 0, 0, clientCfg);
		safecall(clientLogger, saveXMLconfig, simulationId_, pid, 0, 1, trainCfg);
		safecall(clientLogger, saveXMLconfig, simulationId_, pid, 0, 3, engCfg);

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

	sTS2* inferTS; safespawn(inferTS, newsname("inferTS"), defaultdbg, inferCfg, "/TimeSerie");

	sEngine* eng; safespawn(eng, newsname("Engine"), defaultdbg, pid, clientLogger, savedEnginePid_);

	safecall(eng, infer, simulationId_, 0, inferTS, savedEnginePid_);

	timer->stop(endtimeS);
	safecall(clientLogger, saveClientInfo, pid, 0, simulationId_, savedEnginePid_, "Root.Tester", timer->startTime, timer->elapsedTime, "", inferTS->timestamp[1][0], "", false, true, "", "", "", "");

	//-- persist XML config parameters for Client,DataSet
	safecall(clientLogger, saveXMLconfig, simulationId_, pid, 0, 0, clientCfg);
	safecall(clientLogger, saveXMLconfig, simulationId_, pid, 0, 2, inferCfg);

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
	for (int i=0; i<n; i++) (*iBarT)[i]=100;
	VinitRnd(n, -1, 1, (*iBarO));
	VinitRnd(n, -1, 1, (*iBarH));
	VinitRnd(n, -1, 1, (*iBarL));
	VinitRnd(n, -1, 1, (*iBarC));
	VinitRnd(n, -1, 1, (*iBarV));
	for (int i=0; i<n; i++) (*iBaseBarT)[i]=100;
	VinitRnd(n, -1, 1, (*iBaseBarO));
	VinitRnd(n, -1, 1, (*iBaseBarH));
	VinitRnd(n, -1, 1, (*iBaseBarL));
	VinitRnd(n, -1, 1, (*iBaseBarC));
	VinitRnd(n, -1, 1, (*iBaseBarV));
}

void sRoot::kaz() {

	sCfg* tsCfg; safespawn(tsCfg, newsname("tsCfg"), defaultdbg, "Config/inferDSWNN.xml");
	sTS2* ts; safespawn(ts, newsname("newTS"), defaultdbg, tsCfg, "/TimeSerie");
	ts->scale(-1, 1);
	ts->dump();
	return;

	ts->buildDataSet();

//	for(int idx=0; idx<(ts->outputCnt*ts->samplesCnt); idx++) ts->predictionTRS[idx]=ts->targetTRS[idx];
	
	ts->dumpDS();
	ts->slideDS(1);
	ts->dumpDS();
	return;

	ts->getPrediction();
	ts->unscale();
	ts->untransform();
	ts->dump();
	ts->dumpDS();
	return;


	int extraSteps=50;
	int iseriesCnt=3;
	long* ifeatureMask=(long*)malloc(iseriesCnt*sizeof(long)); ifeatureMask[0]=11110; ifeatureMask[1]=01100; ifeatureMask[2]=11110;
	int isampleLen=50;
	int isampleBarsCnt=isampleLen;
	int itargetBarsCnt=3;
	int* iselFcnt=(int*)malloc(iseriesCnt*sizeof(int)); iselFcnt[0]=4; iselFcnt[1]=2; iselFcnt[2]=4;
	int iselFcntTot=iselFcnt[0]*iselFcnt[1]*iselFcnt[2];

	long* iBarT, *iBaseBarT;
	double* iBarO, *iBarH, *iBarL, *iBarC, *iBarV, *iBaseBarO, *iBaseBarH, *iBaseBarL, *iBaseBarC, *iBaseBarV;
	createBars(isampleLen*iselFcntTot, &iBarT, &iBarO, &iBarH, &iBarL, &iBarC, &iBarV, &iBaseBarT, &iBaseBarO, &iBaseBarH, &iBaseBarL, &iBaseBarC, &iBaseBarV);

	int oseriesCnt=1;
	long* ofeatureMask=(long*)malloc(oseriesCnt*sizeof(long)); ofeatureMask[0]=01100;
	int osampleLen=50;
	int osampleBarsCnt=osampleLen;
	int otargetBarsCnt=3;
	int* oselFcnt=(int*)malloc(oseriesCnt*sizeof(int)); oselFcnt[0]=2; 
	int oselFcntTot=oselFcnt[0];

	long* oBarT, *oBaseBarT;
	double *oBarO, *oBarH, *oBarL, *oBarC, *oBarV, *oBaseBarO, *oBaseBarH, *oBaseBarL, *oBaseBarC, *oBaseBarV;
	createBars(osampleLen*oselFcntTot, &oBarT, &oBarO, &oBarH, &oBarL, &oBarC, &oBarV, &oBaseBarT, &oBaseBarO, &oBaseBarH, &oBaseBarL, &oBaseBarC, &oBaseBarV);

//	getForecast(0, extraSteps, iseriesCnt, ifeatureMask, iBarT, iBarO, iBarH, iBarL, iBarC, iBarV, iBaseBarT, iBaseBarO, iBaseBarH, iBaseBarL, iBaseBarC, iBaseBarV, oseriesCnt, ofeatureMask, oBarT, oBarO, oBarH, oBarL, oBarC, oBarV, oBaseBarT, oBaseBarO, oBaseBarH, oBaseBarL, oBaseBarC, oBaseBarV, nullptr, nullptr, nullptr, nullptr, nullptr);
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

#define MT_MAX_SERIES_CNT 12O
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

void sRoot::MTpreprocess(int seriesCnt_, long long* featureMask_, int sampleLen, int sampleBarsCnt, int targetBarsCnt, int** selFcnt, int*** selF, long* BarT, double* BarO, double* BarH, double* BarL, double* BarC, double* BarV, double* BarMACD, double* BarCCI, double* BarATR, double* BarBOLLH, double* BarBOLLM, double* BarBOLLL, double* BarDEMA, double* BarMA, double* BarMOM, long* BaseBarT, double* BaseBarO, double* BaseBarH, double* BaseBarL, double* BaseBarC, double* BaseBarV, double* BaseBarMACD, double* BaseBarCCI, double* BaseBarATR, double* BaseBarBOLLH, double* BaseBarBOLLM, double* BaseBarBOLLL, double* BaseBarDEMA, double* BaseBarMA, double* BaseBarMOM, char*** BarTimeS, char** BarBTimeS, numtype** Bar, numtype** BarB) {
	//-- need to make a local copy of featureMask_, as it gets changed just to get selFcnt
	long long* _featureMask=(long long*)malloc(seriesCnt_*sizeof(long long));
	memcpy_s(_featureMask, seriesCnt_*sizeof(long long), featureMask_, seriesCnt_*sizeof(long long));
	//-- _featureMask to selectedFeature[]
	int selFcntTot=0;
	(*selFcnt)=(int*)malloc(seriesCnt_*sizeof(int));
	(*selF)=(int**)malloc(seriesCnt_*sizeof(int*));
	for (int serie=0; serie<seriesCnt_; serie++) {
		(*selFcnt)[serie]=0;
		(*selF)[serie]=(int*)malloc(14*sizeof(int));
		if (_featureMask[serie]>=10000000000000) { (*selF)[serie][(*selFcnt)[serie]]=FXOPEN; (*selFcnt)[serie]++; _featureMask[serie]-=10000000000000; }	//-- OPEN is selected
		if (_featureMask[serie]>=1000000000000) { (*selF)[serie][(*selFcnt)[serie]]=FXHIGH; (*selFcnt)[serie]++; _featureMask[serie]-=1000000000000; }		//-- HIGH is selected
		if (_featureMask[serie]>=100000000000) { (*selF)[serie][(*selFcnt)[serie]]=FXLOW; (*selFcnt)[serie]++; _featureMask[serie]-=100000000000; }		//-- LOW is selected
		if (_featureMask[serie]>=10000000000) { (*selF)[serie][(*selFcnt)[serie]]=FXCLOSE; (*selFcnt)[serie]++; _featureMask[serie]-=10000000000; }		//-- CLOSE is selected
		if (_featureMask[serie]>=1000000000) { (*selF)[serie][(*selFcnt)[serie]]=FXVOLUME; (*selFcnt)[serie]++; _featureMask[serie]-=1000000000; }			//-- VOLUME is selected
		if (_featureMask[serie]>=100000000) { (*selF)[serie][(*selFcnt)[serie]]=FXMACD; (*selFcnt)[serie]++; _featureMask[serie]-=100000000; }			//-- MACD is selected
		if (_featureMask[serie]>=10000000) { (*selF)[serie][(*selFcnt)[serie]]=FXCCI; (*selFcnt)[serie]++; _featureMask[serie]-=10000000; }			//-- CCI is selected
		if (_featureMask[serie]>=1000000) { (*selF)[serie][(*selFcnt)[serie]]=FXATR; (*selFcnt)[serie]++; _featureMask[serie]-=1000000; }			//-- ATR is selected
		if (_featureMask[serie]>=100000) { (*selF)[serie][(*selFcnt)[serie]]=FXBOLLH; (*selFcnt)[serie]++; _featureMask[serie]-=100000; }			//-- BOLLH is selected
		if (_featureMask[serie]>=10000) { (*selF)[serie][(*selFcnt)[serie]]=FXBOLLM; (*selFcnt)[serie]++; _featureMask[serie]-=10000; }			//-- BOLLM is selected
		if (_featureMask[serie]>=1000) { (*selF)[serie][(*selFcnt)[serie]]=FXBOLLL; (*selFcnt)[serie]++; _featureMask[serie]-=1000; }			//-- BOLLL is selected
		if (_featureMask[serie]>=100) { (*selF)[serie][(*selFcnt)[serie]]=FXDEMA; (*selFcnt)[serie]++; _featureMask[serie]-=100; }			//-- DEMA is selected
		if (_featureMask[serie]>=10) { (*selF)[serie][(*selFcnt)[serie]]=FXMA; (*selFcnt)[serie]++; _featureMask[serie]-=10; }			//-- MA is selected
		if (_featureMask[serie]>=1) { (*selF)[serie][(*selFcnt)[serie]]=FXMOM; (*selFcnt)[serie]++; _featureMask[serie]-=1; }			//-- MOM is selected
		selFcntTot+=(*selFcnt)[serie];
		//info("serie[%d] featuresCnt=%d", serie, (*selFcnt)[serie]);
		//for (int sf=0; sf<(*selFcnt)[serie]; sf++) info("serie[%d] feature [%d]=%d", serie, sf, (*selF)[serie][sf]);
	}

	(*Bar)=(numtype*)malloc((sampleBarsCnt)*selFcntTot*sizeof(numtype));	// flat, ordered by Bar,Feature
	long oBarTime;
	(*BarTimeS)=(char**)malloc((sampleBarsCnt)*sizeof(char*)); for (int b=0; b<(sampleBarsCnt); b++) (*BarTimeS)[b]=(char*)malloc(DATE_FORMAT_LEN);
	int fi=0;
	for (int b=0; b<(sampleBarsCnt); b++) {
		for (int s=0; s<seriesCnt_; s++) {
			oBarTime=BarT[s*sampleLen+b];
			MT4time2str(oBarTime, DATE_FORMAT_LEN, (*BarTimeS)[b]);
			for (int f=0; f<(*selFcnt)[s]; f++) {
				if ((*selF)[s][f]==FXOPEN)   (*Bar)[fi]=(numtype)BarO[s*sampleLen+b];
				if ((*selF)[s][f]==FXHIGH)   (*Bar)[fi]=(numtype)BarH[s*sampleLen+b];
				if ((*selF)[s][f]==FXLOW)    (*Bar)[fi]=(numtype)BarL[s*sampleLen+b];
				if ((*selF)[s][f]==FXCLOSE)  (*Bar)[fi]=(numtype)BarC[s*sampleLen+b];
				if ((*selF)[s][f]==FXVOLUME) (*Bar)[fi]=(numtype)BarV[s*sampleLen+b];
				if ((*selF)[s][f]==FXMACD)    (*Bar)[fi]=(numtype)BarMACD[s*sampleLen+b];
				if ((*selF)[s][f]==FXCCI)    (*Bar)[fi]=(numtype)BarCCI[s*sampleLen+b];
				if ((*selF)[s][f]==FXATR)    (*Bar)[fi]=(numtype)BarATR[s*sampleLen+b];
				if ((*selF)[s][f]==FXBOLLH)    (*Bar)[fi]=(numtype)BarBOLLH[s*sampleLen+b];
				if ((*selF)[s][f]==FXBOLLM)    (*Bar)[fi]=(numtype)BarBOLLM[s*sampleLen+b];
				if ((*selF)[s][f]==FXBOLLL)    (*Bar)[fi]=(numtype)BarBOLLL[s*sampleLen+b];
				if ((*selF)[s][f]==FXDEMA)    (*Bar)[fi]=(numtype)BarDEMA[s*sampleLen+b];
				if ((*selF)[s][f]==FXMA)    (*Bar)[fi]=(numtype)BarMA[s*sampleLen+b];
				if ((*selF)[s][f]==FXMOM)    (*Bar)[fi]=(numtype)BarMOM[s*sampleLen+b];
				fi++;			}
		}
	}
	//--
	(*BarB)=(numtype*)malloc(selFcntTot*sizeof(numtype));
	(*BarBTimeS)=(char*)malloc(DATE_FORMAT_LEN);
	fi=0;
	for (int s=0; s<seriesCnt_; s++) {
		oBarTime=BaseBarT[0];
		MT4time2str(oBarTime, DATE_FORMAT_LEN, (*BarBTimeS));
		for (int f=0; f<(*selFcnt)[s]; f++) {
			if ((*selF)[s][f]==FXOPEN)	  (*BarB)[fi]=(numtype)BaseBarO[s];
			if ((*selF)[s][f]==FXHIGH)   (*BarB)[fi]=(numtype)BaseBarH[s];
			if ((*selF)[s][f]==FXLOW)    (*BarB)[fi]=(numtype)BaseBarL[s];
			if ((*selF)[s][f]==FXCLOSE)  (*BarB)[fi]=(numtype)BaseBarC[s];
			if ((*selF)[s][f]==FXVOLUME) (*BarB)[fi]=(numtype)BaseBarV[s];
			if ((*selF)[s][f]==FXMACD)    (*BarB)[fi]=(numtype)BaseBarMACD[s];
			if ((*selF)[s][f]==FXCCI)    (*BarB)[fi]=(numtype)BaseBarCCI[s];
			if ((*selF)[s][f]==FXATR)    (*BarB)[fi]=(numtype)BaseBarATR[s];
			if ((*selF)[s][f]==FXBOLLH)    (*BarB)[fi]=(numtype)BaseBarBOLLH[s];
			if ((*selF)[s][f]==FXBOLLM)    (*BarB)[fi]=(numtype)BaseBarBOLLM[s];
			if ((*selF)[s][f]==FXBOLLL)    (*BarB)[fi]=(numtype)BaseBarBOLLL[s];
			if ((*selF)[s][f]==FXDEMA)    (*BarB)[fi]=(numtype)BaseBarDEMA[s];
			if ((*selF)[s][f]==FXMA)    (*BarB)[fi]=(numtype)BaseBarMA[s];
			if ((*selF)[s][f]==FXMOM)    (*BarB)[fi]=(numtype)BaseBarMOM[s];
			fi++;
		}
	}
}
void sRoot::getForecast(int seqId_, int predictionStep_, int extraSteps_, int ioShift_, \
	int iseriesCnt_, long long* ifeatureMask_, \
	long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, \
	double* iBarMACD, double* iBarCCI, double* iBarATR, double* iBarBOLLH, double* iBarBOLLM, double* iBarBOLLL, double* iBarDEMA, double* iBarMA, double* iBarMOM, \
	long* iBaseBarT, double* iBaseBarO, double* iBaseBarH, double* iBaseBarL, double* iBaseBarC, double* iBaseBarV, \
	double* iBaseBarMACD, double* iBaseBarCCI, double* iBaseBarATR, double* iBaseBarBOLLH, double* iBaseBarBOLLM, double* iBaseBarBOLLL, double* iBaseBarDEMA, double* iBaseBarMA, double* iBaseBarMOM, \
	int oseriesCnt_, long long* ofeatureMask_, \
	long* oBarT, double* oBarO, double* oBarH, double* oBarL, double* oBarC, double* oBarV, \
	long* oBaseBarT, double* oBaseBarO, double* oBaseBarH, double* oBaseBarL, double* oBaseBarC, double* oBaseBarV, \
	double* oForecastO, double* oForecastH, double* oForecastL, double* oForecastC, double* oForecastV \
) {
	
	int sampleBarsCnt=MT4engine->sampleLen+extraSteps_;
	int targetBarsCnt=MT4engine->targetLen;
//	int sampleBarsCnt=50;// MT4engine->sampleLen+MT4engine->batchSize-1;
//	int targetBarsCnt=3;// MT4engine->targetLen;
//	int sampleLen=50;
//	int batchSize=1;

	char** iBarTimeS; char* iBarBTimeS; int* iselFcnt; int** iselF;
	char** oBarTimeS; char* oBarBTimeS; int* oselFcnt; int** oselF;
	numtype* iBar; numtype* iBarB;
	numtype* oBar; numtype* oBarB;
	MTpreprocess(iseriesCnt_, ifeatureMask_, MT4engine->sampleLen, sampleBarsCnt, targetBarsCnt, &iselFcnt, &iselF, iBarT, iBarO, iBarH, iBarL, iBarC, iBarV, iBarMACD, iBarCCI, iBarATR, iBarBOLLH, iBarBOLLM, iBarBOLLL, iBarDEMA, iBarMA, iBarMOM, iBaseBarT, iBaseBarO, iBaseBarH, iBaseBarL, iBaseBarC, iBaseBarV, iBaseBarMACD, iBaseBarCCI, iBaseBarATR, iBaseBarBOLLH, iBaseBarBOLLM, iBaseBarBOLLL, iBaseBarDEMA, iBaseBarMA, iBaseBarMOM, &iBarTimeS, &iBarBTimeS, &iBar, &iBarB);
	MTpreprocess(oseriesCnt_, ofeatureMask_, MT4engine->sampleLen, sampleBarsCnt, targetBarsCnt, &oselFcnt, &oselF, oBarT, oBarO, oBarH, oBarL, oBarC, oBarV, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, oBaseBarT, oBaseBarO, oBaseBarH, oBaseBarL, oBaseBarC, oBaseBarV, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &oBarTimeS, &oBarBTimeS, &oBar, &oBarB);
	//	MTpreprocess(iseriesCnt_, ifeatureMask_, /*MT4engine->*/sampleLen, sampleBarsCnt, targetBarsCnt, &iselFcnt, &iselF, iBarT, iBarO, iBarH, iBarL, iBarC, iBarV, iBaseBarT, iBaseBarO, iBaseBarH, iBaseBarL, iBaseBarC, iBaseBarV, &iBarTimeS, &iBarBTimeS, &iBar, &iBarB);
//	MTpreprocess(oseriesCnt_, ofeatureMask_, /*MT4engine->*/sampleLen, sampleBarsCnt, targetBarsCnt, &oselFcnt, &oselF, oBarT, oBarO, oBarH, oBarL, oBarC, oBarV, oBaseBarT, oBaseBarO, oBaseBarH, oBaseBarL, oBaseBarC, oBaseBarV, &oBarTimeS, &oBarBTimeS, &oBar, &oBarB);

//	sTS2* mtTS;  safespawn(mtTS, newsname("MTtimeSerie"), defaultdbg, \
//		sampleBarsCnt+targetBarsCnt, dt_, sampleLen, targetBarsCnt, batchSize, MT4doDump, \
//		&iBarTimeS, &iBarBTimeS, iseriesCnt_, iselFcnt, /*MT4engine->WTtype[0], MT4engine->WTlevel[0]*/0, 4, iBar, iBarB, \
//		&oBarTimeS, &oBarBTimeS, oseriesCnt_, oselFcnt, /*MT4engine->WTtype[1], MT4engine->WTlevel[1]*/-1, 0, oBar, oBarB \
//	);

	sTS2* mtTS;  safespawn(mtTS, newsname("MTtimeSerie"), defaultdbg, \
		ioShift_, sampleBarsCnt, MT4engine->dt, MT4engine->sampleLen, MT4engine->targetLen, MT4engine->batchSize, MT4doDump, \
		&iBarTimeS, &iBarBTimeS, iseriesCnt_, iselFcnt, iselF, MT4engine->WTtype[0], MT4engine->WTlevel[0], iBar, iBarB, \
		&oBarTimeS, &oBarBTimeS, oseriesCnt_, oselFcnt, oselF, MT4engine->WTtype[1], MT4engine->WTlevel[1], oBar, oBarB \
	);

	//safecall(mtTS, slide, extraSteps_);
	safecall(MT4engine, infer, MT4accountId, seqId_, mtTS, MT4enginePid);

	for (int s=0; s<(sampleBarsCnt+targetBarsCnt); s++) info("mtTS: %s: act[%d]=%7.6f ; actTR[%d]=%7.6f ; prdTR[%d]=%7.6f ; prd[%d]=%7.6f", mtTS->timestamp[s][1], s, mtTS->val[s][1][0][0][0], s, mtTS->valTR[s][1][0][0][0], s, mtTS->prdTR[s][1][0][0][0], s, mtTS->prd[s][1][0][0][0]);

	for (int i=0; i<MT4engine->outputCnt; i++) {
		oForecastO[i]=0; oForecastH[i]=0; oForecastL[i]=0; oForecastC[i]=0; oForecastV[i]=0;
	}
	int fi=0;
	int frow=(extraSteps_>MT4engine->targetLen) ? (extraSteps_-MT4engine->targetLen+1) : extraSteps_;
	for (int x=0; x<(extraSteps_+1); x++) {
		for (int b=0; b<MT4engine->targetLen; b++) {
			for (int s=0; s<oseriesCnt_; s++) {
				for (int sf=0; sf<oselFcnt[s]; sf++) {

					info("x=%d ; b=%d ; sf=%d ; prdTRS[%d][1][%d][%d][%d]=%7.6f", x, b, sf, MT4engine->sampleLen+x+b, s, sf, 0, mtTS->prdTRS[MT4engine->sampleLen+x+b][1][s][sf][0]);
					info("x=%d ; b=%d ; sf=%d ; prdTR[%d][1][%d][%d][%d]=%7.6f", x, b, sf, MT4engine->sampleLen+x+b, s, sf, 0, mtTS->prdTR[MT4engine->sampleLen+x+b][1][s][sf][0]);
					info("x=%d ; b=%d ; sf=%d ; prd[%d][1][%d][%d][%d]=%7.6f", x, b, sf, MT4engine->sampleLen+x+b, s, sf, 0, mtTS->prd[MT4engine->sampleLen+x+b][1][s][sf][0]);
					
					if (x==extraSteps_ && b==predictionStep_) {
						MT4engine->forecast[fi]=mtTS->prd[MT4engine->sampleLen+x+b][1][s][sf][0];

						if (oselF[s][sf]==FXOPEN) oForecastO[s*MT4engine->targetLen+b]=MT4engine->forecast[fi];
						if (oselF[s][sf]==FXHIGH) oForecastH[s*MT4engine->targetLen+b]=MT4engine->forecast[fi];
						if (oselF[s][sf]==FXLOW) oForecastL[s*MT4engine->targetLen+b]=MT4engine->forecast[fi];
						if (oselF[s][sf]==FXCLOSE) oForecastC[s*MT4engine->targetLen+b]=MT4engine->forecast[fi];
						if (oselF[s][sf]==FXVOLUME) oForecastV[s*MT4engine->targetLen+b]=MT4engine->forecast[fi];
						fi++;
					}
				}
				if(x==extraSteps_) info("OHLCV Forecast, serie %d , bar %d: %7.6f|%7.6f|%7.6f|%7.6f|%7.6f", s, b, oForecastO[s*MT4engine->targetLen+b], oForecastH[s*MT4engine->targetLen+b], oForecastL[s*MT4engine->targetLen+b], oForecastC[s*MT4engine->targetLen+b], oForecastV[s*MT4engine->targetLen+b]);
			}
		}
	}
	safecall(MT4engine, commit);
}
void sRoot::saveTradeInfo(int iPositionTicket, char* iPositionOpenTime, char* iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, double iLastForecastO, double iLastForecastH, double iLastForecastL, double iLastForecastC, double iLastForecastV, char* iCurrBarT, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, int iTradeScenario, int iTradeResult, double iTradeProfit, int iTPhit, int iSLhit) {
	safecall(MT4clientLog, saveTradeInfo, MT4clientPid, MT4sessionId, MT4accountId, MT4enginePid, iPositionTicket, iPositionOpenTime, iLastBarT, iLastBarO, iLastBarH, iLastBarL, iLastBarC, iLastBarV, iLastForecastO, iLastForecastH, iLastForecastL, iLastForecastC, iLastForecastV, iCurrBarT, iForecastO, iForecastH, iForecastL, iForecastC, iForecastV, iTradeScenario, iTradeResult, iTradeProfit, iTPhit, iSLhit);
}
void sRoot::saveClientInfo(int sequenceId, double iPositionOpenTime) {
	char accountStr[64]; sprintf_s(accountStr, 32, "MT5_account_%d", MT4accountId);
	info("MT4enginePid=%d", MT4enginePid);
	safecall(MT4clientLog, saveClientInfo, MT4clientPid, sequenceId, MT4sessionId, MT4enginePid, accountStr, iPositionOpenTime, 0, "", "", "", false, true, "", "", "", "");
}
void sRoot::setMT4env(int clientPid_, int accountId_, int simulationId_, char* clientXMLFile_, int savedEnginePid_, bool doDump_) {
	MT4clientPid=clientPid_;
	MT4accountId=accountId_;
	MT4enginePid=savedEnginePid_;
	MT4doDump=doDump_;
	MT4sessionId=simulationId_;

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

}
void sRoot::MT4createEngine(int* oSampleLen_, int* oPredictionLen_, int* oFeaturesCnt_, int* oBatchSize) {

	//-- check for possible duplicate pid in db (through client persistor), and change it
	safecall(this, getSafePid, MT4clientLog, &MT4clientPid);

	//-- spawn engine from savedEnginePid_ with pid
	safespawn(MT4engine, newsname("Engine"), defaultdbg, MT4clientPid, MT4clientLog, MT4enginePid);

	(*oSampleLen_)=MT4engine->sampleLen;
	(*oPredictionLen_)=MT4engine->targetLen;
	(*oFeaturesCnt_)=MT4engine->inputCnt/MT4engine->sampleLen/(MT4engine->WTlevel[0]+2);
	(*oBatchSize)=MT4engine->batchSize;
	info("Engine spawned from DB. sampleLen=%d ; targetLen=%d ; featuresCnt=%d ; batchSize=%d", (*oSampleLen_), (*oPredictionLen_), (*oFeaturesCnt_), (*oBatchSize));
	info("Environment initialized and Engine created for Account Number %d inferring from Engine pid %d using config from %s", MT4accountId, MT4enginePid, MT4clientXMLFile);
}
void sRoot::MT4commit() {
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
extern "C" __declspec(dllexport) int _createEnv(int accountId_, int simulationId_, char* clientXMLFile_, int savedEnginePid_, bool doDump_, char* oEnvS, int* oSampleLen_, int* oPredictionLen_, int* oFeaturesCnt_, int* oBatchSize_) {
	static sRoot* root;
	try {
		root=new sRoot(nullptr);
		sprintf_s(oEnvS, 64, "%p", root);
		root->setMT4env(GetCurrentProcessId(), accountId_, simulationId_, clientXMLFile_, savedEnginePid_, doDump_);
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
extern "C" __declspec(dllexport) int _getForecast(char* iEnvS, int seqId_, int  predictionStep_, int extraSteps_, int ioShift_, \
	int iseriesCnt_, long long* ifeatureMask_, \
	long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, \
	double* iBarMACD, double* iBarCCI, double* iBarATR, double* iBarBOLLH, double* iBarBOLLM, double* iBarBOLLL, double* iBarDEMA, double* iBarMA, double* iBarMOM, \
	long* iBaseBarT, double* iBaseBarO, double* iBaseBarH, double* iBaseBarL, double* iBaseBarC, double* iBaseBarV, \
	double* iBaseBarMACD, double* iBaseBarCCI, double* iBaseBarATR, double* iBaseBarBOLLH, double* iBaseBarBOLLM, double* iBaseBarBOLLL, double* iBaseBarDEMA, double* iBaseBarMA, double* iBaseBarMOM, \
	int oseriesCnt_, long long* ofeatureMask_, \
	long* oBarT, double* oBarO, double* oBarH, double* oBarL, double* oBarC, double* oBarV, \
	long* oBaseBarT, double* oBaseBarO, double* oBaseBarH, double* oBaseBarL, double* oBaseBarC, double* oBaseBarV, \
	double* oForecastO, double* oForecastH, double* oForecastL, double* oForecastC, double* oForecastV \
) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);

	env->dbg->out(DBG_MSG_INFO, __func__, 0, nullptr, "env=%p . Calling env->getForecast()...", env);
	try {
		env->getForecast(seqId_, predictionStep_, extraSteps_, ioShift_, iseriesCnt_, ifeatureMask_, iBarT, iBarO, iBarH, iBarL, iBarC, iBarV, iBarMACD, iBarCCI, iBarATR, iBarBOLLH, iBarBOLLM, iBarBOLLL, iBarDEMA, iBarMA, iBarMOM, iBaseBarT, iBaseBarO, iBaseBarH, iBaseBarL, iBaseBarC, iBaseBarV, iBaseBarMACD, iBaseBarCCI, iBaseBarATR, iBaseBarBOLLH, iBaseBarBOLLM, iBaseBarBOLLL, iBaseBarDEMA, iBaseBarMA, iBaseBarMOM, oseriesCnt_, ofeatureMask_, oBarT, oBarO, oBarH, oBarL, oBarC, oBarV, oBaseBarT, oBaseBarO, oBaseBarH, oBaseBarL, oBaseBarC, oBaseBarV, oForecastO, oForecastH, oForecastL, oForecastC, oForecastV);
	}
	catch (std::exception exc) {
		return -1;
	}

	return 0;
}
extern "C" __declspec(dllexport) int _saveTradeInfo( \
		char* iEnvS, int iPositionTicket, long iPositionOpenTime, \
		long iLastBarT, double iLastBarO, double iLastBarH, double iLastBarL, double iLastBarC, double iLastBarV, \
		double iLastForecastO, double iLastForecastH, double iLastForecastL, double iLastForecastC, double iLastForecastV, \
		long iCurrBarT, double iForecastO, double iForecastH, double iForecastL, double iForecastC, double iForecastV, \
		int iTradeScenario, int iTradeResult, double iTradeProfit, int iTPhit, int iSLhit \
	) {
	sRoot* env;
	sscanf_s(iEnvS, "%p", &env);
	char iPositionOpenTimes[DATE_FORMAT_LEN];
	char iLastBarTs[DATE_FORMAT_LEN];
	char iCurrBarTs[DATE_FORMAT_LEN];
	
	try {
		//-- first, convert the 2 dates to string
		MT4time2str(iPositionOpenTime, DATE_FORMAT_LEN, iPositionOpenTimes);
		MT4time2str(iLastBarT, DATE_FORMAT_LEN, iLastBarTs);
		MT4time2str(iCurrBarT, DATE_FORMAT_LEN, iCurrBarTs);
		//-- then, make the call
		env->saveTradeInfo(iPositionTicket, iPositionOpenTimes, iLastBarTs, iLastBarO, iLastBarH, iLastBarL, iLastBarC, iLastBarV, iLastForecastO, iLastForecastH, iLastForecastL, iLastForecastC, iLastForecastV, iCurrBarTs, iForecastO, iForecastH, iForecastL, iForecastC, iForecastV, iTradeScenario, iTradeResult, iTradeProfit, iTPhit, iSLhit);
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

	//delete env;

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
