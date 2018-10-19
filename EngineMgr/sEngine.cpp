#include "sEngine.h"

//-- Engine stuff

sEngine::sEngine(sCfgObjParmsDef, sDataShape* dataShape_) : sCfgObj(sCfgObjParmsVal) {

	dataShape=dataShape_;
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
			safespawn(coreLayout[c], newsname("CoreLayout%d", c), defaultdbg, cfg, (newsname("Custom/Core%d/Layout", c))->base, dataShape);
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

DWORD __stdcall coreThreadTrain(LPVOID vargs_) {
	sEngineTrainArgs* args = (sEngineTrainArgs*)vargs_;
	args->core->train(args->coreTrainArgs);
	return 1;
}

void sEngine::train(int testid_, sDataSet* trainDS_) {

	int _pid=GetCurrentProcessId();
	int t;
	int ret = 0;
	int threadsCnt;
	HANDLE* HTrain;
	sEngineTrainArgs** trainArgs;
	void** trainDS;

	HANDLE SMutex = CreateMutex(NULL, FALSE, NULL);

	system("cls");
	for (int l=0; l<layersCnt; l++) {
		
		threadsCnt=layerCoresCnt[l];
		
		//-- initialize layer-level structures
		trainArgs=(sEngineTrainArgs**)malloc(threadsCnt*sizeof(sEngineTrainArgs*));
		trainDS = (void**)malloc(threadsCnt*sizeof(sDataSet*));	
		HTrain = (HANDLE*)malloc(threadsCnt*sizeof(HANDLE));
		DWORD* kaz = (DWORD*)malloc(threadsCnt*sizeof(DWORD));
		LPDWORD* tid = (LPDWORD*)malloc(threadsCnt*sizeof(LPDWORD)); 
		//--
		for (t=0; t<threadsCnt; t++) {
			trainArgs[t]=new sEngineTrainArgs();
			tid[t] = &kaz[t];
			//-- need to make a copy of trainDS for each core running concurrently in the layer
			trainDS[t] = (void*)malloc(sizeof(*trainDS_));
			memcpy_s(trainDS[t], sizeof(*trainDS_), trainDS_, sizeof(*trainDS_));
		}	
		//--

		gotoxy(0, 2+l+((l>0) ? layerCoresCnt[l-1] : 0));  printf("Training Layer %d\n", l);
		t=0;
		for (int c=0; c<coresCnt; c++) {
			if (core[c]->layout->layer==l) {

				//-- rebuild training DataSet for current Core
				((sDataSet*)trainDS[t])->buildFromTS(coreParms[c]->scaleMin[l], coreParms[c]->scaleMax[l]);

				//-- Create Training Thread for current Core
				trainArgs[t]->core=core[c];
				trainArgs[t]->coreTrainArgs->ds = (sDataSet*)trainDS[t];
				trainArgs[t]->coreTrainArgs->screenLine = 2+t+l+((l>0) ? layerCoresCnt[l-1] : 0);

				HTrain[t] = CreateThread(NULL, 0, coreThreadTrain, &(*trainArgs[t]), 0, tid[t]);

				//-- Store Engine Handler
				trainArgs[t]->coreTrainArgs->pid = _pid;
				trainArgs[t]->coreTrainArgs->tid=(*tid[t]);
				trainArgs[t]->coreTrainArgs->testid=testid_;

				//-- associate Training Args to current core
				core[c]->trainArgs=trainArgs[t]->coreTrainArgs;

				t++;
			}
		}
		//-- we need to train all the nets in one layer, in order to have the inputs to the next layer
		WaitForMultipleObjects(t, HTrain, TRUE, INFINITE);

		//-- free(s)
		for (t=0; t<threadsCnt; t++) {
			free(trainArgs[t]); 
			free(trainDS[t]);
		}
		free(trainArgs); free(trainDS); free(HTrain); free(kaz); free(tid);
	}
}
void sEngine::infer(int testid_, sDataSet* testDS_){}
void sEngine::saveMSE() {
	for (int c=0; c<coresCnt; c++) {
		if (core[c]->persistor->saveMSEFlag) safecall(core[c]->persistor, saveMSE, core[c]->trainArgs->pid, core[c]->trainArgs->tid, core[c]->mseCnt, core[c]->mseT, core[c]->mseV);
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
