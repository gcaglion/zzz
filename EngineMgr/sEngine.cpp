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
					core[c]=NNc; coreParms[c]=NNcp;
					break;
				case CORE_GA:
					safespawn(GAcp, newsname("Core%d_GAparms", c), defaultdbg, cfg, (newsname("Custom/Core%d/Parameters", c))->base);
					safespawn(GAc, newsname("Core%d_GA", c), defaultdbg, cfg, "../", coreLayout[c], GAcp);
					core[c]=GAc; coreParms[c]=GAcp;
					break;
				case CORE_SVM:
					safespawn(SVMcp, newsname("Core%d_SVMparms", c), defaultdbg, cfg, (newsname("Custom/Core%d/Parameters", c))->base);
					safespawn(SVMc, newsname("Core%d_SVM", c), defaultdbg, cfg, "../", coreLayout[c], SVMcp);
					core[c]=SVMc;  coreParms[c]=SVMcp;
					break;
				case CORE_SOM:
					safespawn(SOMcp, newsname("Core%d_SOMparms", c), defaultdbg, cfg, (newsname("Custom/Core%d/Parameters", c))->base);
					safespawn(SOMc, newsname("Core%d_SOM", c), defaultdbg, cfg, "../", coreLayout[c], SOMcp);
					core[c]=SOMc; coreParms[c]=SOMcp;
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
//	void** ds;

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
			//-- need to make a copy of ds for each core running concurrently in the layer
			//ds[t] = (void*)malloc(sizeof(*ds_));
			//memcpy_s(ds[t], sizeof(*ds_), ds_, sizeof(*ds_));
		}	
		//--

		gotoxy(0, 2+l+((l>0) ? layerCoresCnt[l-1] : 0));  printf("Training Layer %d\n", l);
		t=0;
		for (int c=0; c<coresCnt; c++) {
			if (core[c]->layout->layer==l) {

				//-- rebuild training DataSet for current Core
				ds_->build(coreParms[c]->scaleMin[l], coreParms[c]->scaleMax[l]);

				//-- Create Training Thread for current Core
				procArgs[t]->coreProcArgs->screenLine = 2+t+l+((l>0) ? layerCoresCnt[l-1] : 0);
				procArgs[t]->core=core[c];
				procArgs[t]->coreProcArgs->ds = (sDataSet*)ds_;
				procArgs[t]->coreProcArgs->runCnt=procArgs[t]->coreProcArgs->ds->samplesCnt;
				procArgs[t]->coreProcArgs->featuresCnt=procArgs[t]->coreProcArgs->ds->selectedFeaturesCnt;
				procArgs[t]->coreProcArgs->feature=procArgs[t]->coreProcArgs->ds->selectedFeature;
				procArgs[t]->coreProcArgs->predictionLen=procArgs[t]->coreProcArgs->ds->predictionLen;
				procArgs[t]->coreProcArgs->targetBFS = procArgs[t]->coreProcArgs->ds->targetBFS;
				procArgs[t]->coreProcArgs->predictionBFS = procArgs[t]->coreProcArgs->ds->predictionBFS;
				procArgs[t]->coreProcArgs->targetSBF = procArgs[t]->coreProcArgs->ds->targetSBF;
				procArgs[t]->coreProcArgs->predictionSBF = procArgs[t]->coreProcArgs->ds->predictionSBF;

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
		for (t=0; t<threadsCnt; t++) {
			free(procArgs[t]); 
			//free(ds[t]);
		}
		free(procArgs); 
		//free(ds); 
		free(procH); free(kaz); free(tid);
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

		//-- 1. convert predictionBFS to predictionSBF
		core[c]->procArgs->ds->reorder(DStarget, BFSorder, SBForder);

		//-- 2. start from sample 1, take step 0 from predictionSBF, copy it into sourceTS->trsvalP
		int Bcnt=core[c]->procArgs->ds->predictionLen;
		int TFcnt=core[c]->procArgs->ds->sourceTS->sourceData->featuresCnt;
		int DFcnt=core[c]->procArgs->ds->selectedFeaturesCnt;
		int Scnt=core[c]->procArgs->ds->samplesCnt;
		int layer=core[c]->layout->layer;

		for (int b=0; b<Bcnt; b++) {
				for (int s=0; s<Scnt; s++) {
					for (int tf=0; tf<TFcnt; tf++) {
						for (int df=0; df<TFcnt; df++) {
							if (core[c]->procArgs->ds->selectedFeature[df]==tf) {

								tsidx=core[c]->procArgs->ds->sampleLen*TFcnt+ s*Bcnt*TFcnt+b*TFcnt+tf;
								dsidx=s*Bcnt*DFcnt+b*DFcnt+df;
								core[c]->procArgs->ds->sourceTS->trsvalP[tsidx] = core[c]->procArgs->predictionSBF[dsidx];

							}
						}
					}
				}
			}
		//-- 3. sourceTS->unscale trsvalP into &trvalP[sampleLen] using scaleM/P already in timeserie
		core[c]->procArgs->ds->sourceTS->unscale(coreParms[c]->scaleMin[layer], coreParms[c]->scaleMax[layer], DFcnt, core[c]->procArgs->ds->selectedFeature, core[c]->procArgs->ds->sourceTS->trsvalP, core[c]->procArgs->ds->sourceTS->trvalP);
		//-- 3.1. do also actual, just to check
		core[c]->procArgs->ds->sourceTS->unscale(coreParms[c]->scaleMin[layer], coreParms[c]->scaleMax[layer], DFcnt, core[c]->procArgs->ds->selectedFeature, core[c]->procArgs->ds->sourceTS->trsvalA, core[c]->procArgs->ds->sourceTS->trvalA);
		//-- 4. sourceTS->untransform into valP
		core[c]->procArgs->ds->sourceTS->untransform(core[c]->procArgs->ds->sourceTS->trvalP, core[c]->procArgs->ds->sourceTS->valP);
		//-- 4.1. do also actual, just to check
		core[c]->procArgs->ds->sourceTS->untransform(core[c]->procArgs->ds->sourceTS->trvalA, core[c]->procArgs->ds->sourceTS->valA);
		//-- persist into runLog
		if (core[c]->persistor->saveRunFlag) safecall(core[c]->persistor, saveRun, core[c]->procArgs->pid, core[c]->procArgs->tid, core[c]->procArgs->npid, core[c]->procArgs->ntid, core[c]->procArgs->runCnt, core[c]->procArgs->featuresCnt, core[c]->procArgs->feature, core[c]->procArgs->predictionLen, core[c]->procArgs->ds->sourceTS->trsvalA, core[c]->procArgs->ds->sourceTS->trsvalP, core[c]->procArgs->ds->sourceTS->trvalA, core[c]->procArgs->ds->sourceTS->trvalP, core[c]->procArgs->ds->sourceTS->valA, core[c]->procArgs->ds->sourceTS->valP);
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
