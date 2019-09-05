#include "sEngine.h"
//#include <vld.h>

//-- Engine stuff
sEngine::sEngine(sObjParmsDef, sLogger* fromPersistor_, int clientPid_, int loadingPid_) : sCfgObj(sObjParmsVal, nullptr, "") {
	
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
	DStrMin=(numtype*)malloc(MAX_TS_FEATURES*sizeof(numtype));
	DStrMax=(numtype*)malloc(MAX_TS_FEATURES*sizeof(numtype));
	DSfftMin=(numtype**)malloc(MAX_WAVELET_LEVELS*sizeof(numtype*)); for (int l=0; l<MAX_WAVELET_LEVELS; l++) DSfftMin[l]=(numtype*)malloc(MAX_TS_FEATURES*sizeof(numtype));
	DSfftMax=(numtype**)malloc(MAX_WAVELET_LEVELS*sizeof(numtype*)); for (int l=0; l<MAX_WAVELET_LEVELS; l++) DSfftMax[l]=(numtype*)malloc(MAX_TS_FEATURES*sizeof(numtype));
	//-- 3. load info from FROM persistor
	safecall(fromPersistor_, loadEngineInfo, loadingPid_, &type, &coresCnt, &sampleLen, &targetLen, &featuresCnt, &batchSize, &WNNdecompLevel, &WNNwaveletType, &persistor->saveToDB, &persistor->saveToFile, persistorDB, coreId, coreType, coreThreadId, coreParentsCnt, coreParent, coreParentConnType, DStrMin, DStrMax, DSfftMin, DSfftMax);
	//-- 2. malloc one core, one coreLayout, one coreParms and one corePersistor for each core
	core=(sCore**)malloc(coresCnt*sizeof(sCore*));
	coreLayout=(sCoreLayout**)malloc(coresCnt*sizeof(sCoreLayout*));
	coreParms=(sCoreParms**)malloc(coresCnt*sizeof(sCoreParms*));
	corePersistor=(sCoreLogger**)malloc(coresCnt*sizeof(sCoreLogger*));
	procArgs=(sEngineProcArgs**)malloc(coresCnt*sizeof(sEngineProcArgs*)); for (int c=0; c<coresCnt; c++) procArgs[c]= new sEngineProcArgs();

	forecast=(numtype*)malloc(targetLen*featuresCnt*sizeof(numtype));

	//-- for each Core, create corePersistor from DB (coreLoggerParms), using loadingPid_ and coreThreadId[]. Also, get layout info, and set base coreLayout properties  (type, desc, connType, outputCnt).
	for (int c=0; c<coresCnt; c++) {
		//corePersistor[c]=new sCoreLogger(this, newsname("CorePersistor%d", c), defaultdbg, GUIreporter, persistor, loadingPid_, coreThreadId[c]);
		safespawn(corePersistor[c], newsname("CorePersistor%d", c), defaultdbg, persistor, loadingPid_, coreThreadId[c]);
		safespawn(coreLayout[c], newsname("CoreLayout%d", c), defaultdbg, sampleLen*featuresCnt, targetLen*featuresCnt, coreType[c], coreParentsCnt[c], coreParent[c], coreParentConnType[c], coreThreadId[c]);
	}

	//-- common to all constructors. once all coreLayouts are created (and all  parents are set), we can determine Layer for each Core, and cores count for each layer
	setLayerProps();

	//-- spawn cores
	safecall(this, spawnCoresFromDB, loadingPid_);

	//-- load cores'images
	safecall(this, loadImage, loadingPid_);

	//-- free(s)
	for (int i=0; i<MAX_ENGINE_CORES; i++) {
		free(coreParent[i]); free(coreParentConnType[i]);
	}
	free(coreParent); free(coreParentConnType); free(coreParentsCnt); free(coreType); free(coreThreadId); free(coreId);
}
sEngine::sEngine(sCfgObjParmsDef, int sampleLen_, int targetLen_, int featuresCnt_, int batchSize_, int clientPid_) : sCfgObj(sCfgObjParmsVal) {

	sampleLen=sampleLen_; targetLen=targetLen_; featuresCnt=featuresCnt_; batchSize=batchSize_;
	forecast=(numtype*)malloc(targetLen*featuresCnt*sizeof(numtype));

	layerCoresCnt=(int*)malloc(MAX_ENGINE_LAYERS*sizeof(int)); for (int l=0; l<MAX_ENGINE_LAYERS; l++) layerCoresCnt[l]=0;
	clientPid=clientPid_;

	//-- engine-level persistor
	safespawn(persistor, newsname("EnginePersistor"), defaultdbg, cfg, "Persistor");

	//-- engine type
	safecall(cfgKey, getParm, &type, "Type");

	//-- cores count
	WNNdecompLevel=-1; WNNwaveletType=-1;
	switch (type) {
	case ENGINE_WNN:
		safecall(cfgKey, getParm, &WNNdecompLevel, "DecompLevel");
		safecall(cfgKey, getParm, &WNNwaveletType, "WaveletType");
		coresCnt=WNNdecompLevel+2;
		break;
	case ENGINE_XIE:
		coresCnt=3;
		break;
	case ENGINE_CUSTOM:
		safecall(cfgKey, getParm, &coresCnt, "CoresCount");
		break;
	default:
		fail("Invalid Engine Type");
	}

	//-- 1. malloc one core, one coreLayout and one coreParms for each core
	core=(sCore**)malloc(coresCnt*sizeof(sCore*));
	coreLayout=(sCoreLayout**)malloc(coresCnt*sizeof(sCoreLayout*));
	coreParms=(sCoreParms**)malloc(coresCnt*sizeof(sCoreParms*));
	procArgs=(sEngineProcArgs**)malloc(coresCnt*sizeof(sEngineProcArgs*)); for (int c=0; c<coresCnt; c++) procArgs[c]= new sEngineProcArgs();

	//-- 2. for each Core, create layout, setting base coreLayout properties  (type, desc, connType, outputCnt)
	for (int c=0; c<coresCnt; c++) {
		safespawn(coreLayout[c], newsname("CoreLayout%d", c), defaultdbg, cfg, strBuild("Core%d/Layout", c), sampleLen*featuresCnt, targetLen*featuresCnt);
	}
	//--
//	mallocTSinfo();

	//-- common to all constructors. once all coreLayouts are created (and all  parents are set), we can determine Layer for each Core, and cores count for each layer
	setLayerProps();

	//-- spawn cores
	spawnCoresFromXML();

	//-- 7. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sEngine::~sEngine() {
	for (int c=0; c<coresCnt; c++) delete procArgs[c];
	free(procArgs);
	free(core); free(coreLayout); free(coreParms);
	free(layerCoresCnt);
	free(forecast);
}

void sEngine::spawnCoresFromXML() {

	//-- spawn each core, layer by layer
	for (int l=0; l<layersCnt; l++) {
		for (int c=0; c<coresCnt; c++) {
			if (coreLayout[c]->layer==l) {
				switch (coreLayout[c]->type) {
				case CORE_NN:
					safespawn(NNcp, newsname("Core%d_NNparms", c), defaultdbg, cfg, strBuild("Core%d/Parameters", c));
					NNcp->setScaleMinMax();
					safespawn(NNc, newsname("Core%d_NN", c), defaultdbg, cfg, "../", coreLayout[c], NNcp);
					coreParms[c]=NNcp; core[c]=NNc;
					break;
				case CORE_GA:
					safespawn(GAcp, newsname("Core%d_GAparms", c), defaultdbg, cfg, strBuild("Core%d/Parameters", c));
					GAcp->setScaleMinMax();
					safespawn(GAc, newsname("Core%d_GA", c), defaultdbg, cfg, "../", coreLayout[c], GAcp);
					coreParms[c]=GAcp; core[c]=GAc;
					break;
				case CORE_SVM:
					safespawn(SVMcp, newsname("Core%d_SVMparms", c), defaultdbg, cfg, strBuild("Core%d/Parameters", c));
					SVMcp->setScaleMinMax();
					safespawn(SVMc, newsname("Core%d_SVM", c), defaultdbg, cfg, "../", coreLayout[c], SVMcp);
					coreParms[c]=SVMcp; core[c]=SVMc;
					break;
				case CORE_SOM:
					safespawn(SOMcp, newsname("Core%d_SOMparms", c), defaultdbg, cfg, strBuild("Core%d/Parameters", c));
					SOMcp->setScaleMinMax();
					safespawn(SOMc, newsname("Core%d_SOM", c), defaultdbg, cfg, "../", coreLayout[c], SOMcp);
					coreParms[c]=SOMcp; core[c]=SOMc;
					break;
				case CORE_DUMB:
					safespawn(DUMBcp, newsname("Core%d_DUMBparms", c), defaultdbg, cfg, strBuild("Core%d/Parameters", c));
					DUMBcp->setScaleMinMax();
					safespawn(DUMBc, newsname("Core%d_DUMB", c), defaultdbg, cfg, "../", coreLayout[c], DUMBcp);
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
					safespawn(NNc, newsname("Core%d_NN", c), defaultdbg, coreLayout[c], corePersistor[c], NNcp);
					//-- 4. set parent classes
					coreParms[c]=NNcp; core[c]=NNc;
					break;
				case CORE_GA:
					//-- 1. core parameters
					safespawn(GAcp, newsname("Core%d_GAparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					GAcp->setScaleMinMax();
					//-- 2. core
					safespawn(GAc, newsname("Core%d_GA", c), defaultdbg, coreLayout[c], corePersistor[c], GAcp);
					//-- 4. set parent classes
					coreParms[c]=GAcp; core[c]=GAc;
					break;
				case CORE_SVM:
					//-- 1. core parameters
					safespawn(SVMcp, newsname("Core%d_SVMparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					SVMcp->setScaleMinMax();
					//-- 2. core
					safespawn(SVMc, newsname("Core%d_SVM", c), defaultdbg, coreLayout[c], corePersistor[c], SVMcp);
					//-- 4. set parent classes
					coreParms[c]=SVMcp; core[c]=SVMc;
					break;
				case CORE_SOM:
					//-- 1. core parameters
					safespawn(SOMcp, newsname("Core%d_SOMparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					SOMcp->setScaleMinMax();
					//-- 2. core
					safespawn(SOMc, newsname("Core%d_SOM", c), defaultdbg, coreLayout[c], corePersistor[c], SOMcp);
					//-- 4. set parent classes
					coreParms[c]=SOMcp; core[c]=SOMc;
					break;
				case CORE_DUMB:
					//-- 1. core parameters
					safespawn(DUMBcp, newsname("Core%d_DUMBparms", c), defaultdbg, persistor, loadingPid, coreLayout[c]->tid);
					DUMBcp->setScaleMinMax();
					//-- 2. core
					safespawn(DUMBc, newsname("Core%d_DUMB", c), defaultdbg, coreLayout[c], corePersistor[c], DUMBcp);
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
		//args->core->Alg->createGPUContext();
		args->core->train(args->coreProcArgs);
		if(!args->core->procArgs->quitAfterBreak) args->core->infer(args->coreProcArgs);
		//args->core->Alg->destroyGPUContext();
	} catch (...) {
		args->coreProcArgs->excp=current_exception();
	}
	return 1;
}
DWORD coreThreadInfer(LPVOID vargs_) {
	sEngineProcArgs* args = (sEngineProcArgs*)vargs_;
	try {
		//args->core->Alg->createGPUContext();
		args->core->infer(args->coreProcArgs);
		//args->core->Alg->destroyGPUContext();
	}
	catch (...) {
		args->coreProcArgs->excp=current_exception();
	}
	return 1;
}
DWORD coreThreadLoad(LPVOID vargs_) {
	sEngineProcArgs* args = (sEngineProcArgs*)vargs_;
	try {
		args->core->loadImage(args->coreProcArgs->npid, args->coreProcArgs->ntid, -1);
	}
	catch (...) {
		args->coreProcArgs->excp=current_exception();
	}
	return 1;
}

void sEngine::loadImage(int loadingPid_) {
	safecall(this, process, loadProc, 0, (sDS**)nullptr, loadingPid_);
	imageLoaded=true;
}
void sEngine::process(int procid_, int testid_, sDS** ds_, int savedEnginePid_) {

	sDS** parentDS;

	int t;
	int ret = 0;
	int threadsCnt;
	HANDLE* procH;
	int lsl0=0;

	system("cls");

	for (int l=0; l<layersCnt; l++) {
		
		threadsCnt=layerCoresCnt[l];
		
		//-- initialize layer-level structures
		procH = (HANDLE*)malloc(threadsCnt*sizeof(HANDLE));
		DWORD* kaz = (DWORD*)malloc(threadsCnt*sizeof(DWORD));
		LPDWORD* tid = (LPDWORD*)malloc(threadsCnt*sizeof(LPDWORD));
		for (t=0; t<threadsCnt; t++) tid[t] = &kaz[t];
		//--

		if (l>0) lsl0+=layerCoresCnt[l-1]+1;
		gotoxy(0, lsl0);  printf("Process %6d, %s Layer %d\n", clientPid, ((procid_==trainProc)?"Training":(procid_==inferProc)?"Inferencing":"Loading Images"), l);
		t=0;
		for (int c=0; c<coresCnt; c++) {
			if (core[c]->layout->layer==l) {

				//-- create dataset for core
				if (ds_!=nullptr) {
					if (l==0) {
						procArgs[c]->coreProcArgs->ds=ds_[c];
					} else {

						parentDS=(sDS**)malloc(coreLayout[c]->parentsCnt*sizeof(sDS*));
						for (int p=0; p<coreLayout[c]->parentsCnt; p++) {
							parentDS[p]=procArgs[coreLayout[c]->parentId[p]]->coreProcArgs->ds;
							safecall(parentDS[p], unscale);
						}

						safespawn(procArgs[c]->coreProcArgs->ds, newsname("Core_%d-%d_Dataset", l, c), defaultdbg, coreLayout[c]->parentsCnt, parentDS);
						safecall(procArgs[c]->coreProcArgs->ds, scale, coreParms[c]->scaleMin[0], coreParms[c]->scaleMax[0]);

						for (int p=0; p<coreLayout[c]->parentsCnt; p++) {
							safecall(parentDS[p], scale, coreParms[coreLayout[c]->parentId[p]]->scaleMin[0], coreParms[coreLayout[c]->parentId[p]]->scaleMax[0]);
						}

						free(parentDS);
					}
					
					//-- set batchCnt
					procArgs[c]->coreProcArgs->batchSize=procArgs[c]->coreProcArgs->ds->batchSize;
					if ((procArgs[c]->coreProcArgs->ds->samplesCnt%procArgs[c]->coreProcArgs->ds->batchSize)!=0) {
						fail("Wrong Batch Size. samplesCnt=%d , batchSamplesCnt=%d", procArgs[c]->coreProcArgs->ds->samplesCnt, procArgs[c]->coreProcArgs->ds->batchSize)
					} else {
						procArgs[c]->coreProcArgs->batchCnt = procArgs[c]->coreProcArgs->ds->samplesCnt/procArgs[c]->coreProcArgs->ds->batchSize;
					}
				}

				//-- Create Training or Infer Thread for current Core
				procArgs[c]->coreProcArgs->screenLine = lsl0+1+t;
				procArgs[c]->core=core[c];
				procArgs[c]->coreProcArgs->pid = clientPid;
				procArgs[c]->coreProcArgs->npid=(savedEnginePid_>0) ? savedEnginePid_ : clientPid;

				if (procid_==loadProc) {
					procArgs[c]->coreProcArgs->batchSize=batchSize;
					procArgs[c]->coreProcArgs->ntid=coreLayout[c]->tid;
					procH[t] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)coreThreadLoad, &(*procArgs[c]), 0, tid[t]);
				} else if (procid_==trainProc) {
					procH[t] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)coreThreadTrain, &(*procArgs[c]), 0, tid[t]);
					procArgs[c]->coreProcArgs->ntid=(*tid[t]);
				} else {
					procH[t] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)coreThreadInfer, &(*procArgs[c]), 0, tid[t]);
					if (clientPid!=procArgs[c]->coreProcArgs->npid) procArgs[c]->coreProcArgs->ntid=coreLayout[c]->tid;
				}
				

				//-- Store Engine Handler
				procArgs[c]->coreProcArgs->tid=(*tid[t]);
				procArgs[c]->coreProcArgs->testid=testid_;

				//-- associate Training Args to current core
				core[c]->procArgs=procArgs[c]->coreProcArgs;

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
		free(procH); free(kaz); free(tid);
	}
}
void sEngine::train(int testid_, sDS** trainDS_) {

	safecall(this, process, trainProc, testid_, trainDS_, 0);
	imageLoaded=true;

	//-- set trmin/max from input

	//-- save training engine, cores, MSE, core images
	if (!core[0]->procArgs->quitAfterBreak) {
		safecall(this, saveInfo);
		for (int c=0; c<coresCnt; c++) {
			if (core[c]->persistor->saveMSEFlag) safecall(core[c]->persistor, saveMSE, core[c]->procArgs->pid, core[c]->procArgs->tid, core[c]->procArgs->mseCnt, core[c]->procArgs->duration, core[c]->procArgs->mseT, core[c]->procArgs->mseV);
			safecall(core[c]->persistor, save, persistor, core[c]->procArgs->pid, core[c]->procArgs->tid);
			if (core[c]->persistor->saveImageFlag) safecall(core[c], saveImage, core[c]->procArgs->pid, core[c]->procArgs->tid, core[c]->procArgs->mseCnt-1);
			free(core[c]->procArgs->mseT);
			free(core[c]->procArgs->mseV);
			free(core[c]->procArgs->duration);
		}
	}
	
}
void sEngine::infer(int testid_, int seqId_, sDS** inferDS_, sTS* inferTS_, int savedEnginePid_) {

	//-- consistency checks: sampleLen/targetLen/featuresCnt must be the same in inferDS and engine
	if (inferDS_[0]->sampleLen!=sampleLen) fail("Infer DataSet Sample Length (%d) differs from Engine's (%d)", inferDS_[0]->sampleLen, sampleLen);
	if (inferDS_[0]->targetLen!=targetLen) fail("Infer DataSet Prediction Length (%d) differs from Engine's (%d)", inferDS_[0]->targetLen, targetLen);
	if (inferDS_[0]->featuresCnt!=featuresCnt) fail("Infer DataSet total features count (%d) differs from Engine's (%d)", inferDS_[0]->featuresCnt, featuresCnt);
	if (inferDS_[0]->batchSize!=batchSize) fail("Infer DataSet Batch Size (%d) differs from Engine's (%d)", inferDS_[0]->batchSize, batchSize);

	//for (int c=0; c<coresCnt; c++) inferDS_[c]->duplicateSequence();
	
	//-- call infer
	safecall(this, process, inferProc, testid_, inferDS_, savedEnginePid_);

	//for (int c=0; c<coresCnt; c++) inferDS_[c]->halveSequence();

	//-- get predicted/target sequences (TR) for all cores, and saveRun
	sDS* _ds;
	int seqLen=inferDS_[0]->samplesCnt+inferDS_[0]->sampleLen+inferDS_[0]->targetLen-1;
	numtype** trgSeqTRS=(numtype**)malloc(coresCnt*sizeof(numtype*));
	numtype** prdSeqTRS=(numtype**)malloc(coresCnt*sizeof(numtype*));
	numtype** trgSeqTR=(numtype**)malloc(coresCnt*sizeof(numtype*));
	numtype** prdSeqTR=(numtype**)malloc(coresCnt*sizeof(numtype*));
	numtype** trgSeqBASE=(numtype**)malloc(coresCnt*sizeof(numtype*));
	numtype** prdSeqBASE=(numtype**)malloc(coresCnt*sizeof(numtype*));
	int c;
	for (c=0; c<coresCnt; c++) {
		_ds=core[c]->procArgs->ds;
		//-- mallocs
		trgSeqTRS[c]=(numtype*)malloc(seqLen*_ds->featuresCnt*sizeof(numtype));
		prdSeqTRS[c]=(numtype*)malloc(seqLen*_ds->featuresCnt*sizeof(numtype));
		trgSeqTR[c]=(numtype*)malloc(seqLen*_ds->featuresCnt*sizeof(numtype));
		prdSeqTR[c]=(numtype*)malloc(seqLen*_ds->featuresCnt*sizeof(numtype));
		trgSeqBASE[c]=(numtype*)malloc(seqLen*_ds->featuresCnt*sizeof(numtype));
		prdSeqBASE[c]=(numtype*)malloc(seqLen*_ds->featuresCnt*sizeof(numtype));
		//--

		_ds->getSeq(TARGET, trgSeqTRS[c], inferDS_[0]); //dumpArrayH(seqLen[c]*_ds->featuresCnt, trgSeqTRS[c], "C:/temp/trgSeqTRS.csv");
		_ds->getSeq(PREDICTION, prdSeqTRS[c], inferDS_[0]); //dumpArrayH(seqLen[c]*_ds->featuresCnt, prdSeqTRS[c], "C:/temp/prdSeqTRS.csv");
		_ds->unscale();
		_ds->getSeq(TARGET, trgSeqTR[c], inferDS_[0]);
		_ds->getSeq(PREDICTION, prdSeqTR[c], inferDS_[0]);
		_ds->untransformSeq(inferTS_->dt, inferTS_->valB, trgSeqTR[c], inferTS_->val, trgSeqBASE[c]);
		_ds->untransformSeq(inferTS_->dt, inferTS_->valB, prdSeqTR[c], inferTS_->val, prdSeqBASE[c]); dumpArrayH(seqLen*_ds->featuresCnt, prdSeqBASE[c], "C:/temp/logs/prdSeqBASE.csv");

		if (core[c]->persistor->saveRunFlag) {
			core[c]->persistor->saveRun(core[c]->procArgs->pid, core[c]->procArgs->tid, core[c]->procArgs->npid, core[c]->procArgs->ntid, seqId_, core[c]->procArgs->mseR, \
				seqLen, inferTS_->timestamp, _ds->featuresCnt, trgSeqTRS[c], prdSeqTRS[c], trgSeqTR[c], prdSeqTR[c], trgSeqBASE[c], prdSeqBASE[c]);
		}
	}

	for (int b=0; b<targetLen; b++) {
		for (int f=0; f<featuresCnt; f++) {
			forecast[b*featuresCnt+f]=prdSeqBASE[c-1][(sampleLen+batchSize-1)*featuresCnt+b*featuresCnt+f];
			info("forecast[%d]=%f", b*featuresCnt+f, forecast[b*featuresCnt+f]);
		}
	}

	for (int c=0; c<coresCnt; c++) {
		free(trgSeqTRS[c]);	free(prdSeqTRS[c]);
		free(trgSeqTR[c]);	free(prdSeqTR[c]);
		free(trgSeqBASE[c]); free(prdSeqBASE[c]);
	}
	free(trgSeqTRS); free(prdSeqTRS);
	free(trgSeqTR); free(prdSeqTR);
	free(trgSeqBASE); free(prdSeqBASE);
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
	int* coreLayer_=(int*)malloc(coresCnt*sizeof(int));
	int* coreType_=(int*)malloc(coresCnt*sizeof(int));
	int* coreThreadId_=(int*)malloc(coresCnt*sizeof(int));
	int* coreParentsCnt_=(int*)malloc(coresCnt*sizeof(int));
	int** coreParent_=(int**)malloc(coresCnt*sizeof(int*));
	int** parentConnType_=(int**)malloc(coresCnt*sizeof(int*));

	//-- set temps
	for (int c=0; c<coresCnt; c++) {
		coreId_[c]=c;
		coreLayer_[c]=coreLayout[c]->layer;
		coreType_[c]=coreLayout[c]->type;
		coreThreadId_[c]=core[c]->procArgs->ntid;
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

	safecall(persistor, saveEngineInfo, \
		clientPid, type, coresCnt, \
		sampleLen, targetLen, featuresCnt, batchSize, \
		WNNdecompLevel, WNNwaveletType, \
		persistor->saveToDB, persistor->saveToFile, persistor->oradb, \
		coreId_, coreLayer_, coreType_, coreThreadId_, coreParentsCnt_, coreParent_, parentConnType_, \
		DStrMin, DStrMax, DSfftMin, DSfftMax
	);

	//-- free temps
	for (int c=0; c<coresCnt; c++) {
		free(coreParent_[c]); free(parentConnType_[c]);
	}
	free(coreId_); free(coreType_); free(coreLayer_); free(coreThreadId_); free(coreParentsCnt_); free(coreParent_); free(parentConnType_);
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
	//-- 4. determine layersCnt
	for (l=0; l<MAX_ENGINE_LAYERS; l++) {
		for (c=0; c<layerCoresCnt[l]; c++) {
		}
		if (c==0) break;
		layersCnt++;
	}
	//-- 5. determine InputCnt for each Core
	for (c=0; c<coresCnt; c++) {
		for (l=0; l<layersCnt; l++) {
			if (coreLayout[c]->layer==l) {
				if (l==0) {
					//-- do nothing. keep core shape same as engine shape
				} else {
					//-- change sampleLen
					coreLayout[c]->inputCnt=layerCoresCnt[l-1]*coreLayout[c]->outputCnt;
				}
			}
		}
	}
}
