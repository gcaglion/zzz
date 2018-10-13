#include "Engine.h"

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
		//-- 1. malloc one coreLayout and one coreParms for each core
		coreLayout=(sCoreLayout**)malloc(coresCnt*sizeof(sCoreLayout*));
		coreParms=(sCoreParms**)malloc(coresCnt*sizeof(sCoreParms*));
		//-- 2. for each Core, create layout and parms, setting base coreLayout properties  (type, desc, connType, outputCnt)
		for (c=0; c<coresCnt; c++) {
			safespawn(false, coreLayout[c], newsname("CoreLayout%d", c), defaultdbg, cfg, (newsname("Custom/CoreLayout%d", c))->base, dataShape);
			safespawn(false, coreLayout[c], newsname("CoreLayout%d", c), defaultdbg, cfg, (newsname("Custom/CoreLayout%d", c))->base, dataShape);
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
				core[c]->baseDataShape->sampleLen=layerCoresCnt[l-1]*core[c]->baseDataShape->predictionLen;
			}
		}
		if (c==0) break;
		layersCnt++;
	}

	//-- 5. spawn each core, layer by layer
	for (l=0; l<layersCnt; l++){
		for (c=0; c<coresCnt; c++) {
			if (core[c]->layout->layer==l) {
				switch (core[c]->type) {
				case CORE_NN:
					//((sNN*)core[c])->init(dataShape, nullptr);
					core[c] = new sNN(this, newsname("Core%d_NN", c), defaultdbg, cfg, (newsname("Custom/Core%d_NN", c))->base, core[c]->layout, dataShape);
					break;
				case CORE_GA:
					//((sGA*)core[c])->init(c, dataShape, nullptr);
					break;
				case CORE_SVM:
					//((sVM*)core[c])->init(c, dataShape, nullptr);
					break;
				case CORE_SOM:
					//((sOM*)core[c])->init(c, dataShape, nullptr);
					break;
				default:
					fail("Invalid Core Type: %d", type);
					break;
				}
			}
		}
	}
	
	//-- 6. restore cfg->currentKey from sCfgObj->bkpKey
	cfg->currentKey=bkpKey;

}

sEngine::~sEngine() {
	free(core);
	free(layerCoresCnt);
}

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

void sEngine::train(sDataSet* trainDS_) {

	//-- 1. 

	for (int l=0; l<layersCnt; l++) {
		for (int c=0; c<coresCnt; c++) {
			if (core[c]->layer==l) {
				switch (core[c]->type) {
				case CORE_NN:
					((sNN*)core[c])->train(trainDS_);
					break;
				case CORE_GA:
					//((sGA*)core[c])->train(trainDS_);
					break;
				case CORE_SVM:
					//((sVM*)core[c])->train(trainDS_);
					break;
				case CORE_SOM:
					//((sOM*)core[c])->train(trainDS_);
					break;
				default:
					fail("Invalid Core Type: %d", type);
					break;
				}
			}
		}
	}
	//-- 1.  
	//-- 2. 
	//-- 3. 
	//-- 4. 
	//-- 5. 
	//-- 6. 
	//-- 7. 
}
void sEngine::infer(sDataSet* testDS_){}

//-- private stuff

