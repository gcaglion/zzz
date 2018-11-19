#include "sEngine.h"

//-- Engine stuff
sEngine::sEngine(sCfgObjParmsDef, int inputCnt_, int outputCnt_, sLogger* fromPersistor_, int loadingPid) : sCfgObj(sCfgObjParmsVal) {

	inputCnt=inputCnt_; outputCnt=outputCnt_;
	layerCoresCnt=(int*)malloc(MAX_ENGINE_LAYERS*sizeof(int)); for (int l=0; l<MAX_ENGINE_LAYERS; l++) layerCoresCnt[l]=0;
	pid=GetCurrentProcessId();

	//-- 0. mallocs
	int* coreId=new int(MAX_ENGINE_CORES);
	int* coreType=new int(MAX_ENGINE_CORES);
	int* coreThreadId=new int(MAX_ENGINE_CORES);
	int* coreParentsCnt=new int(MAX_ENGINE_CORES);
	int** coreParent=(int**)malloc(MAX_ENGINE_CORES*sizeof(int*)); for (int i=0; i<MAX_ENGINE_CORES; i++) coreParent[i]=(int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	int** coreParentConnType=(int**)malloc(MAX_ENGINE_CORES*sizeof(int*)); for (int i=0; i<MAX_ENGINE_CORES; i++) coreParentConnType[i]=(int*)malloc(MAX_ENGINE_CORES*sizeof(int));

	//-- engine-level persistor
	safespawn(persistor, newsname("EnginePersistor"), defaultdbg, cfg, "Persistor");

	//-- 2. load info from FROM persistor
	safecall(fromPersistor_, loadEngineInfo, loadingPid, &type, &coresCnt, coreId, coreType, coreThreadId, coreParentsCnt, coreParent, coreParentConnType);
	if (coresCnt==0) fail("Engine pid %d not found.", loadingPid);

	//-- 3. malloc one core, one coreLayout and one coreParms for each core
	core=(sCore**)malloc(coresCnt*sizeof(sCore*));
	coreLayout=(sCoreLayout**)malloc(coresCnt*sizeof(sCoreLayout*));
	coreParms=(sCoreParms**)malloc(coresCnt*sizeof(sCoreParms*));
	//-- for each Core, get layout info, and set base coreLayout properties  (type, desc, connType, outputCnt)
	for (int c=0; c<coresCnt; c++) {
		safespawn(coreLayout[c], newsname("CoreLayout%d", c), defaultdbg, inputCnt, outputCnt, coreType[c], coreParentsCnt[c], coreParent[c], coreParentConnType[c], coreThreadId[c]);
	}

	//-- common to all constructors. once all coreLayouts are created (and all  parents are set), we can determine Layer for each Core, and cores count for each layer
	setLayerProps();

	//-- spawn cores
	safecall(this, spawnCoresFromDB, loadingPid);

	//-- free(s)
	for (int i=0; i<MAX_ENGINE_CORES; i++) {
		free(coreParent[i]); free(coreParentConnType[i]);
	}
	free(coreParent); free(coreParentConnType); free(coreParentsCnt); free(coreType); free(coreThreadId); free(coreId);
}
sEngine::sEngine(sCfgObjParmsDef, int inputCnt_, int outputCnt_) : sCfgObj(sCfgObjParmsVal) {

	inputCnt=inputCnt_; outputCnt=outputCnt_;
	layerCoresCnt=(int*)malloc(MAX_ENGINE_LAYERS*sizeof(int)); for (int l=0; l<MAX_ENGINE_LAYERS; l++) layerCoresCnt[l]=0;
	pid=GetCurrentProcessId();

	//-- engine-level persistor
	safespawn(persistor, newsname("EnginePersistor"), defaultdbg, cfg, "Persistor");

	//-- engine type
	safecall(cfgKey, getParm, &type, "Type");

	switch (type) {
	case ENGINE_CUSTOM:
		//-- 0. coresCnt
		safecall(cfgKey, getParm, &coresCnt, "Custom/CoresCount");
		//-- 1. malloc one core, one coreLayout and one coreParms for each core
		core=(sCore**)malloc(coresCnt*sizeof(sCore*));
		coreLayout=(sCoreLayout**)malloc(coresCnt*sizeof(sCoreLayout*));
		coreParms=(sCoreParms**)malloc(coresCnt*sizeof(sCoreParms*));
		//-- 2. for each Core, create layout, setting base coreLayout properties  (type, desc, connType, outputCnt)
		for (int c=0; c<coresCnt; c++) {
			safespawn(coreLayout[c], newsname("CoreLayout%d", c), defaultdbg, cfg, (newsname("Custom/Core%d/Layout", c))->base, inputCnt, outputCnt);
		}
		break;
	case ENGINE_WNN:
		//safecall(parms->setKey("WNN"));
		//... get() ...
		break;
	case ENGINE_XIE:
		//safecall(parms->setKey("XIE"));
		//... get() ...
		break;
	default:
		fail("Invalid Engine Type: %d", type);
		break;
	}
	
	//-- common to all constructors. once all coreLayouts are created (and all  parents are set), we can determine Layer for each Core, and cores count for each layer
	setLayerProps();

	//-- spawn cores
	spawnCoresFromXML();

	//-- 7. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sEngine::~sEngine() {
	free(core); free(coreLayout); free(coreParms);
	free(layerCoresCnt);
}

void sEngine::spawnCoresFromXML() {

	//-- spawn each core, layer by layer
	for (int l=0; l<layersCnt; l++) {
		for (int c=0; c<coresCnt; c++) {
			if (coreLayout[c]->layer==l) {
				switch (coreLayout[c]->type) {
				case CORE_NN:
					safespawn(NNcp, newsname("Core%d_NNparms", c), defaultdbg, cfg, (newsname("Custom/Core%d/Parameters", c))->base);
					NNcp->setScaleMinMax();
					safespawn(NNc, newsname("Core%d_NN", c), defaultdbg, cfg, "../", coreLayout[c], NNcp);
					coreParms[c]=NNcp; core[c]=NNc;
					break;
				case CORE_GA:
					safespawn(GAcp, newsname("Core%d_GAparms", c), defaultdbg, cfg, (newsname("Custom/Core%d/Parameters", c))->base);
					GAcp->setScaleMinMax();
					safespawn(GAc, newsname("Core%d_GA", c), defaultdbg, cfg, "../", coreLayout[c], GAcp);
					coreParms[c]=GAcp; core[c]=GAc;
					break;
				case CORE_SVM:
					safespawn(SVMcp, newsname("Core%d_SVMparms", c), defaultdbg, cfg, (newsname("Custom/Core%d/Parameters", c))->base);
					SVMcp->setScaleMinMax();
					safespawn(SVMc, newsname("Core%d_SVM", c), defaultdbg, cfg, "../", coreLayout[c], SVMcp);
					coreParms[c]=SVMcp; core[c]=SVMc;
					break;
				case CORE_SOM:
					safespawn(SOMcp, newsname("Core%d_SOMparms", c), defaultdbg, cfg, (newsname("Custom/Core%d/Parameters", c))->base);
					SOMcp->setScaleMinMax();
					safespawn(SOMc, newsname("Core%d_SOM", c), defaultdbg, cfg, "../", coreLayout[c], SOMcp);
					coreParms[c]=SOMcp; core[c]=SOMc;
					break;
				case CORE_DUMB:
					safespawn(DUMBcp, newsname("Core%d_DUMBparms", c), defaultdbg, cfg, (newsname("Custom/Core%d/Parameters", c))->base);
					DUMBcp->setScaleMinMax();
					safespawn(DUMBc, newsname("Core%d_DUMB", c), defaultdbg, cfg, "../", coreLayout[c], DUMBcp);
					coreParms[c]=DUMBcp; core[c]=DUMBc;
					break;
				default:
					fail("Invalid Core Type: %d", type);
					break;
				}
				cfg->currentKey=cfgKey;
			}
			core[c]->parms=coreParms[c];
		}
	}
}
void sEngine::spawnCoresFromDB(int loadingPid) {

	//-- spawn each core, layer by layer
	for (int l=0; l<layersCnt; l++) {
		for (int c=0; c<coresCnt; c++) {
			if (coreLayout[c]->layer==l) {
				//-- before loading core from db, we ned to find tid it was saved with

				switch (coreLayout[c]->type) {
				case CORE_NN:
					//-- 1. core parameters
					safespawn(NNcp, newsname("Core%d_NNparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					NNcp->setScaleMinMax();
					//-- 2. core
					safespawn(NNc, newsname("Core%d_NN", c), defaultdbg, cfg, (newsname("Custom/Core%d", c))->base, coreLayout[c], NNcp);
					//-- 3. core image
					safecall(NNc, loadImage, loadingPid, coreLayout[c]->tid, -1);
					//-- 4. set parent classes
					coreParms[c]=NNcp; core[c]=NNc;
					break;
				case CORE_GA:
					//-- 1. core parameters
					safespawn(GAcp, newsname("Core%d_GAparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					GAcp->setScaleMinMax();
					//-- 2. core
					safespawn(GAc, newsname("Core%d_GA", c), defaultdbg, cfg, (newsname("Custom/Core%d", c))->base, coreLayout[c], GAcp);
					//-- 3. core image
					safecall(GAc, loadImage, loadingPid, coreLayout[c]->tid, -1);
					//-- 4. set parent classes
					coreParms[c]=GAcp; core[c]=GAc;
					break;
				case CORE_SVM:
					//-- 1. core parameters
					safespawn(SVMcp, newsname("Core%d_SVMparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					SVMcp->setScaleMinMax();
					//-- 2. core
					safespawn(SVMc, newsname("Core%d_SVM", c), defaultdbg, cfg, (newsname("Custom/Core%d", c))->base, coreLayout[c], SVMcp);
					//-- 3. core image
					safecall(SVMc, loadImage, loadingPid, coreLayout[c]->tid, -1);
					//-- 4. set parent classes
					coreParms[c]=SVMcp; core[c]=SVMc;
					break;
				case CORE_SOM:
					//-- 1. core parameters
					safespawn(SOMcp, newsname("Core%d_SOMparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					SOMcp->setScaleMinMax();
					//-- 2. core
					safespawn(SOMc, newsname("Core%d_SOM", c), defaultdbg, cfg, (newsname("Custom/Core%d", c))->base, coreLayout[c], SOMcp);
					//-- 3. core image
					safecall(SOMc, loadImage, loadingPid, coreLayout[c]->tid, -1);
					//-- 4. set parent classes
					coreParms[c]=SOMcp; core[c]=SOMc;
					break;
				case CORE_DUMB:
					//-- 1. core parameters
					safespawn(DUMBcp, newsname("Core%d_DUMBparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					DUMBcp->setScaleMinMax();
					//-- 2. core
					safespawn(DUMBc, newsname("Core%d_DUMB", c), defaultdbg, cfg, (newsname("Custom/Core%d", c))->base, coreLayout[c], DUMBcp);
					//-- 3. core image
					safecall(DUMBc, loadImage, loadingPid, coreLayout[c]->tid, -1);
					//-- 4. set parent classes
					coreParms[c]=DUMBcp; core[c]=DUMBc;
					break;
				default:
					fail("Invalid Core Type: %d", type);
					break;
				}
				core[c]->parms=coreParms[c];
			}
		}
	}
}

DWORD coreThreadTrain(LPVOID vargs_) {
	sEngineProcArgs* args = (sEngineProcArgs*)vargs_;
	args->core->train(args->coreProcArgs);
	return 1;
}
DWORD coreThreadInfer(LPVOID vargs_) {
	sEngineProcArgs* args = (sEngineProcArgs*)vargs_;
	sDataSet* ds = args->coreProcArgs->ds;
	args->core->infer(ds->samplesCnt, ds->sampleLen, ds->predictionLen, ds->selectedFeaturesCnt, ds->sampleSBF, ds->targetSBF, ds->predictionSBF);
	return 1;
}

void sEngine::process(int procid_, int testid_, sDataSet* ds_, int savedEnginePid_) {

	int t;
	int ret = 0;
	int threadsCnt;
	HANDLE* procH;
	sEngineProcArgs** procArgs;

	HANDLE SMutex = CreateMutex(NULL, FALSE, NULL);

	system("cls");
	for (int l=0; l<layersCnt; l++) {
		
		threadsCnt=layerCoresCnt[l];
		
		//-- initialize layer-level structures
		procArgs=(sEngineProcArgs**)malloc(threadsCnt*sizeof(sEngineProcArgs*));
		//ds = (void**)malloc(threadsCnt*sizeof(sDataSet*));	
		procH = (HANDLE*)malloc(threadsCnt*sizeof(HANDLE));
		DWORD* kaz = (DWORD*)malloc(threadsCnt*sizeof(DWORD));
		LPDWORD* tid = (LPDWORD*)malloc(threadsCnt*sizeof(LPDWORD)); 
		//--
		for (t=0; t<threadsCnt; t++) {
			procArgs[t]=new sEngineProcArgs();
			tid[t] = &kaz[t];
		}	
		//--

		gotoxy(0, 2+l+((l>0) ? layerCoresCnt[l-1] : 0));  printf("Process %6d, %s Layer %d\n", pid, ((procid_==trainProc)?"Training":"Inferencing"), l);
		t=0;
		for (int c=0; c<coresCnt; c++) {
			if (core[c]->layout->layer==l) {

				//-- scale trdata and rebuild training DataSet for current Core
				ds_->sourceTS->scale(TARGET, TR, coreParms[c]->scaleMin[l], coreParms[c]->scaleMax[l]);
				ds_->build(TARGET, TRS);
				

				//-- Create Training or Infer Thread for current Core
				procArgs[t]->coreProcArgs->screenLine = 2+t+l+((l>0) ? layerCoresCnt[l-1] : 0);
				procArgs[t]->core=core[c];
				procArgs[t]->coreProcArgs->ds = (sDataSet*)ds_;
				procArgs[t]->coreProcArgs->npid=savedEnginePid_;
				procArgs[t]->coreProcArgs->tsFeaturesCnt=procArgs[t]->coreProcArgs->ds->sourceTS->sourceData->featuresCnt;
				procArgs[t]->coreProcArgs->selectedFeaturesCnt=procArgs[t]->coreProcArgs->ds->selectedFeaturesCnt;
				procArgs[t]->coreProcArgs->selectedFeature=procArgs[t]->coreProcArgs->ds->selectedFeature;
				procArgs[t]->coreProcArgs->predictionLen=procArgs[t]->coreProcArgs->ds->predictionLen;
				procArgs[t]->coreProcArgs->targetBFS = procArgs[t]->coreProcArgs->ds->targetBFS;
				procArgs[t]->coreProcArgs->predictionBFS = procArgs[t]->coreProcArgs->ds->predictionBFS;
				procArgs[t]->coreProcArgs->targetSBF = procArgs[t]->coreProcArgs->ds->targetSBF;
				procArgs[t]->coreProcArgs->predictionSBF = procArgs[t]->coreProcArgs->ds->predictionSBF;

				if (procid_==trainProc) {
					procH[t] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)coreThreadTrain, &(*procArgs[t]), 0, tid[t]);
				} else {
					procH[t] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)coreThreadInfer, &(*procArgs[t]), 0, tid[t]);
				}

				//-- Store Engine Handler
				procArgs[t]->coreProcArgs->pid = pid;
				procArgs[t]->coreProcArgs->tid=(*tid[t]);
				procArgs[t]->coreProcArgs->testid=testid_;

				//-- associate Training Args to current core
				core[c]->procArgs=procArgs[t]->coreProcArgs;

				t++;
			}
		}
		//-- we need to train all the nets in one layer, in order to have the inputs to the next layer
		WaitForMultipleObjects(t, procH, TRUE, INFINITE);

		//-- free(s)
		for (t=0; t<threadsCnt; t++) free(procArgs[t]); 
		free(procArgs); free(procH); free(kaz); free(tid);
	}
}
void sEngine::train(int testid_, sDataSet* trainDS_) {
	safecall(this, process, trainProc, testid_, trainDS_, 0);
}
void sEngine::infer(int testid_, sDataSet* inferDS_, int savedEnginePid_) {
	safecall(this, process, inferProc, testid_, inferDS_, savedEnginePid_);
}

void sEngine::saveMSE() {
	for (int c=0; c<coresCnt; c++) {
		if (core[c]->persistor->saveMSEFlag) safecall(core[c]->persistor, saveMSE, core[c]->procArgs->pid, core[c]->procArgs->tid, core[c]->procArgs->mseCnt, core[c]->procArgs->mseT, core[c]->procArgs->mseV);
	}
}
void sEngine::saveImage(int epoch) {

	//-- cores Images
	for (int c=0; c<coresCnt; c++) {
		if (core[c]->persistor->saveImageFlag) safecall(core[c], saveImage, core[c]->procArgs->pid, core[c]->procArgs->tid, epoch);
	}
}
void sEngine::saveRun() {
	for (int c=0; c<coresCnt; c++) {

		sDataSet* _ds = core[c]->procArgs->ds;
		sTimeSerie* _ts = _ds->sourceTS;
		int layer=core[c]->layout->layer;

		//-- 2. take step 0 from predictionSBF, copy it into sourceTS->trsvalP
		_ds->unbuild(PREDICTED, PREDICTED, TRS);
		if(_ts->doDump) _ts->dump(PREDICTED, TRS);

		//-- 3. sourceTS->unscale trsvalP into &trvalP[sampleLen] using scaleM/P already in timeserie
		_ts->unscale(PREDICTED, coreParms[c]->scaleMin[layer], coreParms[c]->scaleMax[layer], _ds->selectedFeaturesCnt, _ds->selectedFeature, _ds->sampleLen);
		if (_ts->doDump) _ts->dump(PREDICTED, TR);

		//-- 5. sourceTS->untransform into valP
		_ds->sourceTS->untransform(PREDICTED, PREDICTED, core[c]->procArgs->ds->sampleLen, core[c]->procArgs->ds->selectedFeaturesCnt, core[c]->procArgs->ds->selectedFeature);
		if (_ts->doDump) _ts->dump(PREDICTED, BASE);

		//-- persist into runLog
		int runStepsCnt=core[c]->procArgs->ds->samplesCnt +core[c]->procArgs->ds->sampleLen;
		if (core[c]->persistor->saveRunFlag) core[c]->persistor->saveRun(core[c]->procArgs->pid, core[c]->procArgs->tid, core[c]->procArgs->npid, core[c]->procArgs->ntid, runStepsCnt, core[c]->procArgs->tsFeaturesCnt, core[c]->procArgs->selectedFeaturesCnt, core[c]->procArgs->selectedFeature, core[c]->procArgs->predictionLen, _ts->dtime, _ts->val[TARGET][TRS], _ts->val[PREDICTED][TRS], _ts->val[TARGET][TR], _ts->val[PREDICTED][TR], _ts->val[TARGET][BASE], _ts->val[PREDICTED][BASE], _ts->barWidth );
	}
}
void sEngine::commit() {
	for (int c=0; c<coresCnt; c++) {
		safecall(core[c]->persistor, commit);
	}
	safecall(this->persistor, commit);
}

void sEngine::saveInfo() {

	//-- malloc temps
	int* coreId_=(int*)malloc(coresCnt*sizeof(int));
	int* coreType_=(int*)malloc(coresCnt*sizeof(int));
	int* coreThreadId_=(int*)malloc(coresCnt*sizeof(int));
	int* coreParentsCnt_=(int*)malloc(coresCnt*sizeof(int));
	int** coreParent_=(int**)malloc(coresCnt*sizeof(int*));
	int** parentConnType_=(int**)malloc(coresCnt*sizeof(int*));

	//-- set temps
	for (int c=0; c<coresCnt; c++) {
		coreId_[c]=c;
		coreType_[c]=coreLayout[c]->type;
		coreThreadId_[c]=core[c]->procArgs->tid;
		coreParentsCnt_[c]=coreLayout[c]->parentsCnt;
		coreParent_[c]=(int*)malloc(coreParentsCnt_[c]*sizeof(int));
		parentConnType_[c]=(int*)malloc(coreParentsCnt_[c]*sizeof(int));
		for (int p=0; p<coreParentsCnt_[c]; p++) {
			coreParent_[c][p]=p;
			parentConnType_[c][p]=core[c]->layout->parentConnType[p];
		}
		//-- for each core, save core Parameters
		safecall(core[c]->parms, save, persistor, pid, coreThreadId_[c]);
	}

	//-- actual call
	safecall(persistor, saveEngineInfo, pid, type, coresCnt, coreId_, coreType_, coreThreadId_, coreParentsCnt_, coreParent_, parentConnType_);

	//-- free temps
	for (int c=0; c<coresCnt; c++) {
		free(coreParent_[c]); free(parentConnType_[c]);
	}
	free(coreId_); free(coreType_); free(coreThreadId_); free(coreParentsCnt_); free(coreParent_); free(parentConnType_);
}

//-- private stuff
void sEngine::setCoreLayer(sCoreLayout* cl) {
	int ret=0;
	int maxParentLayer=-1;
	for (int p=0; p<cl->parentsCnt; p++) {
		sCoreLayout* parent=coreLayout[cl->parentId[p]];
		setCoreLayer(parent);
		if (parent->layer>maxParentLayer) {
			maxParentLayer=parent->layer;
		}
		ret=maxParentLayer+1;
	}
	cl->layer=ret;
}
void sEngine::setLayerProps() {
	int l, c;
	for (c=0; c<coresCnt; c++) {
		setCoreLayer(coreLayout[c]);
		layerCoresCnt[coreLayout[c]->layer]++;
	}
	//-- 4. determine layersCnt, and InputCnt for each Core
	for (l=0; l<MAX_ENGINE_LAYERS; l++) {
		for (c=0; c<layerCoresCnt[l]; c++) {
			if (l==0) {
				//-- do nothing. keep core shape same as engine shape
			} else {
				//-- change sampleLen
				coreLayout[c]->inputCnt=layerCoresCnt[l-1]*coreLayout[c]->outputCnt;
			}
		}
		if (c==0) break;
		layersCnt++;
	}
}
