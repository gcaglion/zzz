#include "sEngine.h"

//-- Engine stuff

sEngine::sEngine(sCfgObjParmsDef, sDataShape* dataShape_) : sCfgObj(sCfgObjParmsVal) {

	dataShape=dataShape_;
	layerCoresCnt=(int*)malloc(MAX_ENGINE_LAYERS*sizeof(int)); for (int l=0; l<MAX_ENGINE_LAYERS; l++) layerCoresCnt[l]=0;

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
		coreLogger=(sCoreLogger**)malloc(coresCnt*sizeof(sCoreLogger*));
		//-- 2. for each Core, create persistor and layout, setting base coreLayout properties  (type, desc, connType, outputCnt)
		for (c=0; c<coresCnt; c++) {
			safespawn(coreLayout[c], newsname("CoreLayout%d", c), defaultdbg, cfg, (newsname("Custom/Core%d/Layout", c))->base, dataShape);
			safespawn(coreLogger[c], newsname("CorePersistor%d", c), defaultdbg, cfg, (newsname("Custom/Core%d/Persistor", c))->base);
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
				coreLayout[c]->shape->sampleLen=layerCoresCnt[l-1]*coreLayout[c]->shape->predictionLen;
			}
		}
		if (c==0) break;
		layersCnt++;
	}

	//-- 6. spawn each core, layer by layer
	sNN* NNc; sGA* GAc; sSVM* SVMc; sSOM* SOMc;
	sNNparms* NNcp; sGAparms* GAcp; sSVMparms* SVMcp; sSOMparms* SOMcp;
	//-- 1. spawn core parameters
	//-- 2. set core parameters' scaleMin/Max
	//-- 3. spawn core
	//-- note: levelsCnt must always be set when creating specific core parameter object (NNcp, GAcp, ...)
	for (l=0; l<layersCnt; l++){
		for (c=0; c<coresCnt; c++) {
			if (coreLayout[c]->layer==l) {
				switch (coreLayout[c]->type) {
				case CORE_NN:
					safespawn(NNcp, newsname("Core%d_NNparms", c), defaultdbg, cfg, (newsname("Custom/Core%d/Parameters", c))->base);
					NNcp->setScaleMinMax();
					safespawn(NNc, newsname("Core%d_NN", c), defaultdbg, cfg, "../", coreLayout[c], NNcp, coreLogger[c]);
					core[c]=NNc; coreParms[c]=NNcp;
					break;
				case CORE_GA:
					safespawn(GAcp, newsname("Core%d_GAparms", c), defaultdbg, cfg, (newsname("Custom/Core%d/Parameters", c))->base);
					safespawn(GAc, newsname("Core%d_GA", c), defaultdbg, cfg, "../", coreLayout[c], GAcp, coreLogger[c]);
					core[c]=GAc; coreParms[c]=GAcp;
					break;
				case CORE_SVM:
					safespawn(SVMcp, newsname("Core%d_SVMparms", c), defaultdbg, cfg, (newsname("Custom/Core%d/Parameters", c))->base);
					safespawn(SVMc, newsname("Core%d_SVM", c), defaultdbg, cfg, "../", coreLayout[c], SVMcp, coreLogger[c]);
					core[c]=SVMc;  coreParms[c]=SVMcp;
					break;
				case CORE_SOM:
					safespawn(SOMcp, newsname("Core%d_SOMparms", c), defaultdbg, cfg, (newsname("Custom/Core%d/Parameters", c))->base);
					safespawn(SOMc, newsname("Core%d_SOM", c), defaultdbg, cfg, "../", coreLayout[c], SOMcp, coreLogger[c]);
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


void sEngine::train(int simulationId_, sDataSet* trainDS_) {

	//-- 1. 

	for (int l=0; l<layersCnt; l++) {
		for (int c=0; c<coresCnt; c++) {
			if (core[c]->layout->layer==l) {
				trainDS_->build(coreParms[c]->scaleMin[l], coreParms[c]->scaleMax[l]);
				safecall(core[c], train, trainDS_);
			}
		}
	}

}
void sEngine::infer(int simulationId_, sDataSet* testDS_){}
void sEngine::saveMSE() {
	for (int c=0; c<coresCnt; c++) if(core[c]->persistor->saveMSEFlag) core[c]->persistor->saveMSE(core[c]->pid, core[c]->tid, core[c]->mseCnt, core[c]->mseT, core[c]->mseV);
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
void sEngine::layerTrain(int pid, int pTestId, int pLayer, bool loadW, sDataSet* trainDS_, sTrainParams* tp) {
	int t;
	int ret = 0;
	int ThreadCount = layerCoresCnt[pLayer];
	HANDLE* HTrain = (HANDLE*)malloc(ThreadCount*sizeof(HANDLE));
	DWORD* kaz = (DWORD*)malloc(ThreadCount*sizeof(DWORD));
	LPDWORD* tid = (LPDWORD*)malloc(ThreadCount*sizeof(LPDWORD)); for (int i = 0; i < ThreadCount; i++) tid[i] = &kaz[i];
	HANDLE SMutex = CreateMutex(NULL, FALSE, NULL);

	gotoxy(0, 2+pLayer+((pLayer>0) ? layerCoresCnt[pLayer-1] : 0));  printf("Training Layer %d\n", pLayer);
	t = 0;
	for (int n = 0; n<layerCoresCnt[pLayer]; n++) {
		tp[t].LayerId = pLayer;
		tp[t].CoreId = n;
		tp[t].CorePos = 2+t+pLayer+((pLayer>0) ? layerCoresCnt[pLayer-1] : 0);
		tp[t].ScreenMutex = SMutex;
		tp[t].TotCores = layerCoresCnt[pLayer];
		tp[t].SampleCount = trainDS_->samplesCnt;
		tp[t].useValidation = false;	//******************************//
		tp[t].useExistingW = loadW;

		//-- Create Thread
		HTrain[t] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)core[n], &tp[t], 0, tid[t]);

		//-- Store Engine Handler
		tp[t].TrainInfo.ProcessId = pid;
		tp[t].TrainInfo.TestId = pTestId;
		tp[t].TrainInfo.ThreadId = (*tid[t]);
		//pEngineParms->Core[pLayer][n].CoreLog[d].ThreadId = (*tid[t]);
		t++;
	}
	//-- we need to train all the nets in one layer, in order to have the inputs to the next layer
	WaitForMultipleObjects(t, HTrain, TRUE, INFINITE);

	//-- check for training failure
	for (int ti = 0; ti<t; ti++) if (tp[ti].TrainSuccess!=0) ret = -1;

	//-- free(s)
	free(HTrain); free(kaz); free(tid);

}