#include "sEngine.h"

void sEngine::mallocTSinfo() {
	TSfeaturesCnt=(int*)malloc(DATASET_MAX_SOURCETS_CNT*sizeof(int));
	TSfeature=(int**)malloc(DATASET_MAX_SOURCETS_CNT*sizeof(int*)); for (int ts=0; ts<DATASET_MAX_SOURCETS_CNT; ts++) TSfeature[ts]=(int*)malloc(MAX_DATA_FEATURES*sizeof(int));
	TStrMin=(numtype**)malloc(DATASET_MAX_SOURCETS_CNT*sizeof(numtype*)); for (int ts=0; ts<DATASET_MAX_SOURCETS_CNT; ts++) TStrMin[ts]=(numtype*)malloc(MAX_DATA_FEATURES*sizeof(numtype));
	TStrMax=(numtype**)malloc(DATASET_MAX_SOURCETS_CNT*sizeof(numtype*)); for (int ts=0; ts<DATASET_MAX_SOURCETS_CNT; ts++) TStrMax[ts]=(numtype*)malloc(MAX_DATA_FEATURES*sizeof(numtype));
}
void sEngine::freeTSinfo() {
	for (int ts=0; ts<DATASET_MAX_SOURCETS_CNT; ts++) {
		free(TSfeature[ts]); 
		free(TStrMin[ts]); 
		free(TStrMax[ts]);
	}
	free(TSfeature); free(TStrMin); free(TStrMax); free(TSfeaturesCnt);
}

//-- Engine stuff
sEngine::sEngine(sObjParmsDef, sLogger* fromPersistor_, int clientPid_, int loadingPid_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	
	//-- init Algebra / CUDA/CUBLAS/CURAND stuff
	safespawn(Alg, newsname("%s_Algebra", name->base), defaultdbg);

	safespawn(shape, newsname("%s_DataShape", name->base), defaultdbg, 0, 0, 0);
	sOraData* persistorDB;
	safespawn(persistorDB, newsname("%s_Logger_DB", name->base), defaultdbg,"","","");
	safespawn(persistor, newsname("%s_Logger", name->base), defaultdbg, persistorDB);

	layerCoresCnt=(int*)malloc(MAX_ENGINE_LAYERS*sizeof(int)); for (int l=0; l<MAX_ENGINE_LAYERS; l++) layerCoresCnt[l]=0;
	clientPid=clientPid_;

	//-- 0. mallocs
	int* coreId=(int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	int* coreType=(int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	int* coreThreadId=(int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	int* coreParentsCnt=(int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	int** coreParent=(int**)malloc(MAX_ENGINE_CORES*sizeof(int*)); for (int i=0; i<MAX_ENGINE_CORES; i++) coreParent[i]=(int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	int** coreParentConnType=(int**)malloc(MAX_ENGINE_CORES*sizeof(int*)); for (int i=0; i<MAX_ENGINE_CORES; i++) coreParentConnType[i]=(int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	//--
	mallocTSinfo();

	//-- 3. load info from FROM persistor
	int typeUNUSED;
	safecall(fromPersistor_, loadEngineInfo, loadingPid_, &typeUNUSED, &coresCnt, &shape->sampleLen, &shape->predictionLen, &shape->featuresCnt, &persistor->saveToDB, &persistor->saveToFile, persistorDB, coreId, coreType, coreThreadId, coreParentsCnt, coreParent, coreParentConnType, &sourceTSCnt, TSfeaturesCnt, TSfeature, TStrMin, TStrMax);
	//-- 2. malloc one core, one coreLayout, one coreParms and one corePersistor for each core
	core=(sCore**)malloc(coresCnt*sizeof(sCore*));
	coreLayout=(sCoreLayout**)malloc(coresCnt*sizeof(sCoreLayout*));
	coreParms=(sCoreParms**)malloc(coresCnt*sizeof(sCoreParms*));
	corePersistor=(sCoreLogger**)malloc(coresCnt*sizeof(sCoreLogger*));

	//-- for each Core, create corePersistor from DB (coreLoggerParms), using loadingPid_ and coreThreadId[]. Also, get layout info, and set base coreLayout properties  (type, desc, connType, outputCnt).
	for (int c=0; c<coresCnt; c++) {
		//corePersistor[c]=new sCoreLogger(this, newsname("CorePersistor%d", c), defaultdbg, GUIreporter, persistor, loadingPid_, coreThreadId[c]);
		safespawn(corePersistor[c], newsname("CorePersistor%d", c), defaultdbg, persistor, loadingPid_, coreThreadId[c]);
		safespawn(coreLayout[c], newsname("CoreLayout%d", c), defaultdbg, shape->sampleLen*shape->featuresCnt, shape->predictionLen*shape->featuresCnt, coreType[c], coreParentsCnt[c], coreParent[c], coreParentConnType[c], coreThreadId[c]);
	}

	//-- common to all constructors. once all coreLayouts are created (and all  parents are set), we can determine Layer for each Core, and cores count for each layer
	setLayerProps();

	//-- spawn cores
	safecall(this, spawnCoresFromDB, loadingPid_);

	//-- re-transform sourceTSs

	//-- free(s)
	for (int i=0; i<MAX_ENGINE_CORES; i++) {
		free(coreParent[i]); free(coreParentConnType[i]);
	}
	free(coreParent); free(coreParentConnType); free(coreParentsCnt); free(coreType); free(coreThreadId); free(coreId);
}
sEngine::sEngine(sCfgObjParmsDef, sDataShape* shape_, int clientPid_) : sCfgObj(sCfgObjParmsVal) {

	shape=shape_;
	layerCoresCnt=(int*)malloc(MAX_ENGINE_LAYERS*sizeof(int)); for (int l=0; l<MAX_ENGINE_LAYERS; l++) layerCoresCnt[l]=0;
	clientPid=clientPid_;

	//-- init Algebra / CUDA/CUBLAS/CURAND stuff
	safespawn(Alg, newsname("%s_Algebra", name->base), dbg);

	//-- engine-level persistor
	safespawn(persistor, newsname("EnginePersistor"), defaultdbg, cfg, "Persistor");

	//-- 0. coresCnt
	safecall(cfgKey, getParm, &coresCnt, "CoresCount");
	//-- 1. malloc one core, one coreLayout and one coreParms for each core
	core=(sCore**)malloc(coresCnt*sizeof(sCore*));
	coreLayout=(sCoreLayout**)malloc(coresCnt*sizeof(sCoreLayout*));
	coreParms=(sCoreParms**)malloc(coresCnt*sizeof(sCoreParms*));
	//-- 2. for each Core, create layout, setting base coreLayout properties  (type, desc, connType, outputCnt)
	for (int c=0; c<coresCnt; c++) {
		safespawn(coreLayout[c], newsname("CoreLayout%d", c), defaultdbg, cfg, (newsname("Core%d/Layout", c))->base, shape->sampleLen*shape->featuresCnt, shape->predictionLen*shape->featuresCnt);
	}
	//--
	mallocTSinfo();

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
	freeTSinfo();
}

void sEngine::spawnCoresFromXML() {

	//-- spawn each core, layer by layer
	for (int l=0; l<layersCnt; l++) {
		for (int c=0; c<coresCnt; c++) {
			if (coreLayout[c]->layer==l) {
				switch (coreLayout[c]->type) {
				case CORE_NN:
					safespawn(NNcp, newsname("Core%d_NNparms", c), defaultdbg, cfg, (newsname("Core%d/Parameters", c))->base);
					NNcp->setScaleMinMax();
					safespawn(NNc, newsname("Core%d_NN", c), defaultdbg, cfg, "../", Alg, coreLayout[c], NNcp);
					coreParms[c]=NNcp; core[c]=NNc;
					break;
				case CORE_GA:
					safespawn(GAcp, newsname("Core%d_GAparms", c), defaultdbg, cfg, (newsname("Core%d/Parameters", c))->base);
					GAcp->setScaleMinMax();
					safespawn(GAc, newsname("Core%d_GA", c), defaultdbg, cfg, "../", Alg, coreLayout[c], GAcp);
					coreParms[c]=GAcp; core[c]=GAc;
					break;
				case CORE_SVM:
					safespawn(SVMcp, newsname("Core%d_SVMparms", c), defaultdbg, cfg, (newsname("Core%d/Parameters", c))->base);
					SVMcp->setScaleMinMax();
					safespawn(SVMc, newsname("Core%d_SVM", c), defaultdbg, cfg, "../", Alg, coreLayout[c], SVMcp);
					coreParms[c]=SVMcp; core[c]=SVMc;
					break;
				case CORE_SOM:
					safespawn(SOMcp, newsname("Core%d_SOMparms", c), defaultdbg, cfg, (newsname("Core%d/Parameters", c))->base);
					SOMcp->setScaleMinMax();
					safespawn(SOMc, newsname("Core%d_SOM", c), defaultdbg, cfg, "../", Alg, coreLayout[c], SOMcp);
					coreParms[c]=SOMcp; core[c]=SOMc;
					break;
				case CORE_DUMB:
					safespawn(DUMBcp, newsname("Core%d_DUMBparms", c), defaultdbg, cfg, (newsname("Core%d/Parameters", c))->base);
					DUMBcp->setScaleMinMax();
					safespawn(DUMBc, newsname("Core%d_DUMB", c), defaultdbg, cfg, "../", Alg, coreLayout[c], DUMBcp);
					coreParms[c]=DUMBcp; core[c]=DUMBc;
					break;
				default:
					fail("Invalid Core Type: %d", coreLayout[c]->type);
					break;
				}
				cfg->currentKey=cfgKey;
				core[c]->parms=coreParms[c];
			}
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
					safespawn(NNc, newsname("Core%d_NN", c), defaultdbg, Alg, coreLayout[c], corePersistor[c], NNcp);
					//-- 4. set parent classes
					coreParms[c]=NNcp; core[c]=NNc;
					break;
				case CORE_GA:
					//-- 1. core parameters
					safespawn(GAcp, newsname("Core%d_GAparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					GAcp->setScaleMinMax();
					//-- 2. core
					safespawn(GAc, newsname("Core%d_GA", c), defaultdbg, Alg, coreLayout[c], corePersistor[c], GAcp);
					//-- 4. set parent classes
					coreParms[c]=GAcp; core[c]=GAc;
					break;
				case CORE_SVM:
					//-- 1. core parameters
					safespawn(SVMcp, newsname("Core%d_SVMparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					SVMcp->setScaleMinMax();
					//-- 2. core
					safespawn(SVMc, newsname("Core%d_SVM", c), defaultdbg, Alg, coreLayout[c], corePersistor[c], SVMcp);
					//-- 4. set parent classes
					coreParms[c]=SVMcp; core[c]=SVMc;
					break;
				case CORE_SOM:
					//-- 1. core parameters
					safespawn(SOMcp, newsname("Core%d_SOMparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					SOMcp->setScaleMinMax();
					//-- 2. core
					safespawn(SOMc, newsname("Core%d_SOM", c), defaultdbg, Alg, coreLayout[c], corePersistor[c], SOMcp);
					//-- 4. set parent classes
					coreParms[c]=SOMcp; core[c]=SOMc;
					break;
				case CORE_DUMB:
					//-- 1. core parameters
					safespawn(DUMBcp, newsname("Core%d_DUMBparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					DUMBcp->setScaleMinMax();
					//-- 2. core
					safespawn(DUMBc, newsname("Core%d_DUMB", c), defaultdbg, Alg, coreLayout[c], corePersistor[c], DUMBcp);
					//-- 4. set parent classes
					coreParms[c]=DUMBcp; core[c]=DUMBc;
					break;
				default:
					fail("Invalid Core Type: %d", coreLayout[c]->type);
					break;
				}
				core[c]->parms=coreParms[c];
			}
		}
	}
}

DWORD coreThreadTrain(LPVOID vargs_) {
	sEngineProcArgs* args = (sEngineProcArgs*)vargs_;
	try {
		args->core->train(args->coreProcArgs);
	} catch (...) {
		args->coreProcArgs->excp=current_exception();
	}
	return 1;
}
DWORD coreThreadInfer(LPVOID vargs_) {
	sEngineProcArgs* args = (sEngineProcArgs*)vargs_;
	args->core->infer(args->coreProcArgs);
	return 1;
}

void sEngine::train2(int testid_, sDS* sampleDS_, sDS* targetDS_, sDS* predictionDS_, int batchSize_) {
	sDS** parentDSt;
	sDS** parentDSp;

	//-- spawn sample/target/prediction DSs for each core
	sDS** coreDSs=(sDS**)malloc(coresCnt*sizeof(sDS*));
	sDS** coreDSt=(sDS**)malloc(coresCnt*sizeof(sDS*));
	sDS** coreDSp=(sDS**)malloc(coresCnt*sizeof(sDS*));
	for (int l=0; l<layersCnt; l++) {
		for (int c=0; c<coresCnt; c++) {
			if (coreLayout[c]->layer==l) {
				if (l==0) {
					safespawn(coreDSs[c], newsname("Core_%d-%d_Samples_Dataset", l, c), defaultdbg, 1, &sampleDS_);
					safespawn(coreDSt[c], newsname("Core_%d-%d_Targets_Dataset", l, c), defaultdbg, 1, &targetDS_);
					safespawn(coreDSp[c], newsname("Core_%d-%d_Predictions_Dataset", l, c), defaultdbg, 1, &predictionDS_);
				} else {
					parentDSt=(sDS**)malloc(coreLayout[c]->parentsCnt*sizeof(sDS*));
					parentDSp=(sDS**)malloc(coreLayout[c]->parentsCnt*sizeof(sDS*));
					//--
					for (int p=0; p<coreLayout[c]->parentsCnt; p++) {
						parentDSt[p]=coreDSt[coreLayout[c]->parentId[p]];
						parentDSp[p]=coreDSp[coreLayout[c]->parentId[p]];
					}
					safespawn(coreDSt[c], newsname("Core_%d-%d_Targets_Dataset", l, c), defaultdbg, coreLayout[c]->parentsCnt, parentDSt);
					safespawn(coreDSp[c], newsname("Core_%d-%d_Predictions_Dataset", l, c), defaultdbg, coreLayout[c]->parentsCnt, parentDSp);
					//--
					free(parentDSt);
					free(parentDSp);
				}
			}
		}
	}
	//-- 


}
void sEngine::process(int procid_, bool loadImage_, int testid_, sDataSet* ds_, int batchSize_, int savedEnginePid_) {

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

		gotoxy(0, 2+l+((l>0) ? layerCoresCnt[l-1] : 0));  printf("Process %6d, %s Layer %d\n", clientPid, ((procid_==trainProc)?"Training":"Inferencing"), l);
		t=0;
		for (int c=0; c<coresCnt; c++) {
			if (core[c]->layout->layer==l) {

				safecall(ds_, build, ACTUAL, BASE);
				//-- scale trdata and rebuild training DataSet for current Core
				for(int t=0; t<ds_->sourceTScnt; t++) safecall(ds_->sourceTS[t], scale, ACTUAL, TR, coreParms[c]->scaleMin[l], coreParms[c]->scaleMax[l]);
				safecall(ds_, build, ACTUAL, TRS);

				//-- Create Training or Infer Thread for current Core
				procArgs[t]->coreProcArgs->screenLine = 2+t+l+((l>0) ? layerCoresCnt[l-1] : 0);
				procArgs[t]->core=core[c];
				procArgs[t]->coreProcArgs->ds = (sDataSet*)ds_;
				procArgs[t]->coreProcArgs->batchSize=batchSize_;
				procArgs[t]->coreProcArgs->loadImage=loadImage_;
				procArgs[t]->coreProcArgs->npid=savedEnginePid_;
				procArgs[t]->coreProcArgs->ntid=coreLayout[c]->tid;

				if (procid_==trainProc) {
					procH[t] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)coreThreadTrain, &(*procArgs[t]), 0, tid[t]);
				} else {
					procH[t] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)coreThreadInfer, &(*procArgs[t]), 0, tid[t]);
				}

				//-- Store Engine Handler
				procArgs[t]->coreProcArgs->pid = clientPid;
				procArgs[t]->coreProcArgs->tid=(*tid[t]);
				procArgs[t]->coreProcArgs->testid=testid_;

				//-- associate Training Args to current core
				core[c]->procArgs=procArgs[t]->coreProcArgs;

				t++;
			}
		}
		//-- we need to train all the nets in one layer, in order to have the inputs to the next layer
		WaitForMultipleObjects(t, procH, TRUE, INFINITE);

		//-- exception handling for threads
		int ti;
		for (ti=0; ti<t; ti++) {
			if (procArgs[ti]->coreProcArgs->excp!=NULL) rethrow_exception(procArgs[ti]->coreProcArgs->excp);
		}

		//-- free(s)
		for (t=0; t<threadsCnt; t++) free(procArgs[t]); 
		free(procArgs); free(procH); free(kaz); free(tid);
	}
}
void sEngine::train(int testid_, sDataSet* trainDS_) {

	//-- needed to set trmin/max for each training feature
	trainDS=trainDS_;

	safecall(this, process, trainProc, false, testid_, trainDS_, trainDS_->batchSamplesCnt, 0);
}
void sEngine::infer(int testid_, sDataSet* inferDS_, int savedEnginePid_, bool reTransform) {

	//-- consistency checks: sampleLen/predictionLen/featuresCnt must be the same in inferDS and engine
	if (inferDS_->shape->sampleLen!=shape->sampleLen) fail("Infer DataSet Sample Length (%d) differs from Engine's (%d)", inferDS_->shape->sampleLen, shape->sampleLen);
	if (inferDS_->shape->predictionLen!=shape->predictionLen) fail("Infer DataSet Prediction Length (%d) differs from Engine's (%d)", inferDS_->shape->predictionLen, shape->predictionLen);
	if (inferDS_->shape->featuresCnt!=shape->featuresCnt) fail("Infer DataSet total features count (%d) differs from Engine's (%d)", inferDS_->shape->featuresCnt, shape->featuresCnt);

	//-- re-transform inferDS using trMin/Max loaded
	if (reTransform) {
		for (int ts=0; ts<inferDS_->sourceTScnt; ts++) {
			for (int tsf=0; tsf<inferDS_->sourceTS[ts]->sourceData->featuresCnt; tsf++) {
				for (int selF=0; selF<inferDS_->selectedTSfeaturesCnt[ts]; selF++) {
					if (inferDS_->selectedTSfeature[ts][selF]==tsf) {
						inferDS_->sourceTS[ts]->dmin[tsf]=TStrMin[ts][selF];
						inferDS_->sourceTS[ts]->dmax[tsf]=TStrMax[ts][selF];
						inferDS_->sourceTS[ts]->transform(ACTUAL);
					}
				}
			}
		}
	}
	//-- call infer
	safecall(this, process, inferProc, reTransform, testid_, inferDS_, 1, savedEnginePid_);

	//-- unscale, untransform, then save results
	sDataSet* _ds;
	sTimeSerie* _ts;
	int layer;
	for (int c=0; c<coresCnt; c++) {
		layer=core[c]->layout->layer;
		_ds = core[c]->procArgs->ds;

		for (int t=0; t<_ds->sourceTScnt; t++) {
			_ts = _ds->sourceTS[t];

			//-- 2. take step 0 from predictionSBF, copy it into sourceTS->trsvalP
			safecall(_ds, unbuild, PREDICTED, PREDICTED, TRS);
			if (_ts->doDump) _ts->dump(PREDICTED, TRS);

			//-- 3. sourceTS->unscale trsvalP into &trvalP[sampleLen] using scaleM/P already in timeserie
			safecall(_ts, unscale, PREDICTED, coreParms[c]->scaleMin[layer], coreParms[c]->scaleMax[layer], _ds->selectedTSfeaturesCnt[t], _ds->selectedTSfeature[t], _ds->shape->sampleLen);
			if (_ts->doDump) _ts->dump(PREDICTED, TR);

			//-- 5. sourceTS->untransform into valP
			safecall(_ts, untransform, PREDICTED);
			if (_ts->doDump) _ts->dump(PREDICTED, BASE);
		}
	}

}

void sEngine::saveMSE() {
	for (int c=0; c<coresCnt; c++) {
		if (core[c]->persistor->saveMSEFlag) safecall(core[c]->persistor, saveMSE, core[c]->procArgs->pid, core[c]->procArgs->tid, core[c]->procArgs->mseCnt, core[c]->procArgs->duration, core[c]->procArgs->mseT, core[c]->procArgs->mseV);
	}
}
void sEngine::saveCoreLoggers() {
	for (int c=0; c<coresCnt; c++) safecall(core[c]->persistor, save, persistor, core[c]->procArgs->pid, core[c]->procArgs->tid);
}
void sEngine::saveCoreImages(int epoch) {
	for (int c=0; c<coresCnt; c++) if (core[c]->persistor->saveImageFlag) safecall(core[c], saveImage, core[c]->procArgs->pid, core[c]->procArgs->tid, (epoch==-1)? core[c]->procArgs->mseCnt-1:epoch);
}
void sEngine::saveRun() {

	sDataSet* _ds;
	sTimeSerie* _ts;
	int layer;
	int runStepsCnt;

	for (int c=0; c<coresCnt; c++) {
		layer=core[c]->layout->layer;
		_ds = core[c]->procArgs->ds;

		for (int t=0; t<_ds->sourceTScnt; t++) {
			_ts = _ds->sourceTS[t];

			//-- persist into runLog
			runStepsCnt= _ds->samplesCnt + _ds->shape->sampleLen + _ds->shape->predictionLen -1;

			if (core[c]->persistor->saveRunFlag) {
				core[c]->persistor->saveRun(core[c]->procArgs->pid, core[c]->procArgs->tid, core[c]->procArgs->npid, core[c]->procArgs->ntid, core[c]->procArgs->mseR, \
						runStepsCnt, t, _ts->sourceData->featuresCnt, _ds->selectedTSfeaturesCnt[t], _ds->selectedTSfeature[t], _ds->shape->predictionLen, \
						_ts->dtime, _ts->val[ACTUAL][TRS], _ts->val[PREDICTED][TRS], _ts->val[ACTUAL][TR], _ts->val[PREDICTED][TR], _ts->val[ACTUAL][BASE], _ts->val[PREDICTED][BASE], _ts->barWidth);
			}
		}
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
		safecall(core[c]->parms, save, persistor, clientPid, coreThreadId_[c]);
	}

	//-- save trMin/Max for every selected feature in every TS
	for (int ts=0; ts<trainDS->sourceTScnt; ts++) {
		for (int tsf=0; tsf<trainDS->sourceTS[ts]->sourceData->featuresCnt; tsf++) {
			for (int selF=0; selF<trainDS->selectedTSfeaturesCnt[ts]; selF++) {
				if (trainDS->selectedTSfeature[ts][selF]==tsf) {
					TStrMin[ts][selF]=trainDS->sourceTS[ts]->dmin[tsf];
					TStrMax[ts][selF]=trainDS->sourceTS[ts]->dmax[tsf];
				}
			}
		}
	}

	//-- actual call
	safecall(persistor, saveEngineInfo, clientPid, -1, shape->sampleLen, shape->predictionLen, shape->featuresCnt, coresCnt, persistor->saveToDB, persistor->saveToFile, persistor->oradb, \
		coreId_, coreType_, coreThreadId_, coreParentsCnt_, coreParent_, parentConnType_, \
		trainDS->sourceTScnt, trainDS->selectedTSfeaturesCnt, trainDS->selectedTSfeature, TStrMin, TStrMax);

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
