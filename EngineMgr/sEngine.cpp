#include "sEngine.h"

//-- Engine stuff

sEngine::sEngine(sCfgObjParmsDef, int inputCnt_, int outputCnt_) : sCfgObj(sCfgObjParmsVal) {

	inputCnt=inputCnt_; outputCnt=outputCnt_;
	layerCoresCnt=(int*)malloc(MAX_ENGINE_LAYERS*sizeof(int)); for (int l=0; l<MAX_ENGINE_LAYERS; l++) layerCoresCnt[l]=0;
	pid=GetCurrentProcessId();

	//-- 1. get Parameters
	safecall(cfgKey, getParm, &type, "Type");

	//-- 2. do stuff and spawn sub-Keys
	int l, c;

	switch (type) {
	case ENGINE_CUSTOM:
		//-- 0. coresCnt
		safecall(cfgKey, getParm, &coresCnt, "Custom/CoresCount");
		//-- 1. malloc one core, one coreLayout and one coreParms for each core
		core=(sCore**)malloc(coresCnt*sizeof(sCore*));
		coreLayout=(sCoreLayout**)malloc(coresCnt*sizeof(sCoreLayout*));
		coreParms=(sCoreParms**)malloc(coresCnt*sizeof(sCoreParms*));
		//-- 2. for each Core, create persistor and layout, setting base coreLayout properties  (type, desc, connType, outputCnt)
		for (c=0; c<coresCnt; c++) {
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

	//-- 3. once all coreLayouts are created (and all  parents are set), we can determine Layer for each Core, and cores count for each layer
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

	//-- 6. spawn each core, layer by layer
	sNN* NNc; sGA* GAc; sSVM* SVMc; sSOM* SOMc;
	sNNparms* NNcp; sGAparms* GAcp; sSVMparms* SVMcp; sSOMparms* SOMcp;
	for (l=0; l<layersCnt; l++){
		for (c=0; c<coresCnt; c++) {
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
				default:
					fail("Invalid Core Type: %d", type);
					break;
				}
				cfg->currentKey=cfgKey;
			}
		}
	}
	
	//-- 7. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}
sEngine::~sEngine() {
	free(core); free(coreLayout); free(coreParms);
	free(layerCoresCnt);
}

DWORD coreThreadTrain(LPVOID vargs_) {
	sEngineProcArgs* args = (sEngineProcArgs*)vargs_;
	args->core->train(args->coreProcArgs);
	return 1;
}
DWORD coreThreadInfer(LPVOID vargs_) {
	sEngineProcArgs* args = (sEngineProcArgs*)vargs_;
	args->core->infer(args->coreProcArgs);
	return 1;
}

void sEngine::process(int procid_, int testid_, sDataSet* ds_) {

	int _pid=GetCurrentProcessId();
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
				ds_->build(BASE);
				ds_->build(TR);
				ds_->sourceTS->scale(coreParms[c]->scaleMin[l], coreParms[c]->scaleMax[l]);
				ds_->sourceTS->dump(TRS, TARGET);
				ds_->build(TRS);
				

				//-- Create Training or Infer Thread for current Core
				procArgs[t]->coreProcArgs->screenLine = 2+t+l+((l>0) ? layerCoresCnt[l-1] : 0);
				procArgs[t]->core=core[c];
				procArgs[t]->coreProcArgs->ds = (sDataSet*)ds_;
				procArgs[t]->coreProcArgs->tsFeaturesCnt=procArgs[t]->coreProcArgs->ds->sourceTS->sourceData->featuresCnt;
				procArgs[t]->coreProcArgs->selectedFeaturesCnt=procArgs[t]->coreProcArgs->ds->selectedFeaturesCnt;
				procArgs[t]->coreProcArgs->selectedFeature=procArgs[t]->coreProcArgs->ds->selectedFeature;
				procArgs[t]->coreProcArgs->predictionLen=procArgs[t]->coreProcArgs->ds->predictionLen;
				procArgs[t]->coreProcArgs->targetBFS = procArgs[t]->coreProcArgs->ds->targetBFS[TRS];
				procArgs[t]->coreProcArgs->predictionBFS = procArgs[t]->coreProcArgs->ds->predictionBFS[TRS];
				procArgs[t]->coreProcArgs->targetSBF = procArgs[t]->coreProcArgs->ds->targetSBF[TRS];
				procArgs[t]->coreProcArgs->predictionSBF = procArgs[t]->coreProcArgs->ds->predictionSBF[TRS];

				if (procid_==trainProc) {
					procH[t] = CreateThread(NULL, 0, coreThreadTrain, &(*procArgs[t]), 0, tid[t]);
				} else {
					procH[t] = CreateThread(NULL, 0, coreThreadInfer, &(*procArgs[t]), 0, tid[t]);
				}

				//-- Store Engine Handler
				procArgs[t]->coreProcArgs->pid = _pid;
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
	process(trainProc, testid_, trainDS_);
}
void sEngine::infer(int testid_, sDataSet* inferDS_) {
	process(inferProc, testid_, inferDS_);
}
void sEngine::saveMSE() {
	for (int c=0; c<coresCnt; c++) {
		if (core[c]->persistor->saveMSEFlag) safecall(core[c]->persistor, saveMSE, core[c]->procArgs->pid, core[c]->procArgs->tid, core[c]->procArgs->mseCnt, core[c]->procArgs->mseT, core[c]->procArgs->mseV);
	}
}
void sEngine::saveRun() {
	int dsidx;
	int tsidx;
	for (int c=0; c<coresCnt; c++) {

		//-- 1. convert predictionBFS to predictionSBF (done 
		core[c]->procArgs->ds->reorder(DSprediction, BFSorder, SBForder);

		//-- 2. take step 0 from predictionSBF, copy it into sourceTS->trsvalP
		int Bcnt=core[c]->procArgs->ds->predictionLen;
		int TFcnt=core[c]->procArgs->ds->sourceTS->sourceData->featuresCnt;
		int DFcnt=core[c]->procArgs->ds->selectedFeaturesCnt;
		int* selF=core[c]->procArgs->ds->selectedFeature;
		int Scnt=core[c]->procArgs->ds->samplesCnt;
		int layer=core[c]->layout->layer;
		for (int b=0; b<Bcnt; b++) {
				for (int s=0; s<Scnt; s++) {
					for (int tf=0; tf<TFcnt; tf++) {
						for (int df=0; df<DFcnt; df++) {
							if (core[c]->procArgs->ds->selectedFeature[df]==tf) {

								tsidx=s*Bcnt*TFcnt+b*TFcnt+tf;
								dsidx=s*Bcnt*DFcnt+b*DFcnt+selF[df];
								if (s>0) {
									core[c]->procArgs->ds->sourceTS->trsvalP[tsidx] = core[c]->procArgs->predictionSBF[dsidx];
								} else {
									core[c]->procArgs->ds->sourceTS->trsvalP[tsidx] =EMPTY_VALUE;
								}

							}
						}
					}
				}
			}
		
		//-- 3. sourceTS->unscale trsvalP into &trvalP[sampleLen] using scaleM/P already in timeserie
		sTimeSerie* _ts = core[c]->procArgs->ds->sourceTS;
		sDataSet* _ds = core[c]->procArgs->ds;
		_ts->unscale(coreParms[c]->scaleMin[layer], coreParms[c]->scaleMax[layer], DFcnt, _ds->selectedFeature, _ds->sampleLen, PREDICTED);
		
		//-- 4. copy trvalA into trvalP for the first <sampleLen> bars, so we have a baseval
		size_t leftsz=core[c]->procArgs->ds->sampleLen*core[c]->procArgs->ds->sourceTS->sourceData->featuresCnt*sizeof(numtype);
		memcpy_s(core[c]->procArgs->ds->sourceTS->trvalP, leftsz, core[c]->procArgs->ds->sourceTS->trvalA, leftsz);
		//-- 5. sourceTS->untransform into valP
		core[c]->procArgs->ds->sourceTS->untransform(core[c]->procArgs->ds->selectedFeaturesCnt, core[c]->procArgs->ds->selectedFeature, core[c]->procArgs->ds->sourceTS->trvalP, core[c]->procArgs->ds->sourceTS->valP);
		
		//-- persist into runLog
		int runStepsCnt=core[c]->procArgs->ds->samplesCnt +core[c]->procArgs->ds->sampleLen;
		if (core[c]->persistor->saveRunFlag) safecall(\
			core[c]->persistor, saveRun, core[c]->procArgs->pid, core[c]->procArgs->tid, core[c]->procArgs->npid, core[c]->procArgs->ntid, \
			runStepsCnt, core[c]->procArgs->tsFeaturesCnt, core[c]->procArgs->selectedFeaturesCnt, core[c]->procArgs->selectedFeature, core[c]->procArgs->predictionLen, \
			core[c]->procArgs->ds->sourceTS->trsvalA, core[c]->procArgs->ds->sourceTS->trsvalP, \
			core[c]->procArgs->ds->sourceTS->trvalA, core[c]->procArgs->ds->sourceTS->trvalP, \
			core[c]->procArgs->ds->sourceTS->valA, core[c]->procArgs->ds->sourceTS->valP \
		);
	}
}
void sEngine::commit() {
	for (int c=0; c<coresCnt; c++) {
		safecall(core[c]->persistor, commit);
	}
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
