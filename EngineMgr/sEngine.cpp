#include "sEngine.h"

void sEngine::mallocs() {
	coreType = (int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	coreThreadId = (int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	coreLayer = (int*)malloc(MAX_ENGINE_CORES*sizeof(int));
	core = (sCore**)malloc(MAX_ENGINE_CORES*sizeof(sCore*));
}
sEngine::sEngine(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	mallocs();
	pid=GetCurrentProcessId();
	safecall(cfgKey, getParm, &coresCnt, "CoresCount");
	safespawn(persistor, newsname("EnginePersistor"), defaultdbg, cfg, "Persistor");

}
sEngine::sEngine(sObjParmsDef, sLogger* persistor_, int clientPid_, int savedEnginePid_) : sCfgObj(sObjParmsVal, nullptr, "") {
	mallocs();
	pid=clientPid_;
	safecall(persistor_, loadEngineCoresInfo, savedEnginePid_, &coresCnt, &coreType, &coreThreadId, &coreLayer);
	safecall(persistor_, loadEngineInfo, savedEnginePid_, &type, &sampleLen, &targetLen, &batchSize, &WTlevel[0], &WTtype[0]);

	sNNparms* NNcp=nullptr; sNN2* NNc=nullptr; sCoreLogger* NNcl;
	for (int c=0; c<coresCnt; c++) {
		safecall(persistor_, loadCoreInfo, savedEnginePid_, coreThreadId[c], coreType, &sampleLen, &inputCnt, &targetLen, &outputCnt, &batchSize, &trMinIN, &trMaxIN, &trMinOUT, &trMaxOUT);
		safespawn(NNcl, newsname("NNcPersistor"), defaultdbg, persistor_, savedEnginePid_, coreThreadId[c]);
		safespawn(NNcp, newsname("NNcParms"), defaultdbg, NNcl, savedEnginePid_, coreThreadId[c]);
		safespawn(NNc, newsname("inferNNcore"), defaultdbg, inputCnt, outputCnt, NNcl, NNcp);
		core[c]=NNc;
		core[c]->persistor=NNcl;
		core[c]->parms=NNcp; core[c]->parms->setScaleMinMax();

		safecall(NNc, loadImage, savedEnginePid_, coreThreadId[c], -1);
	}

}
sEngine::~sEngine(){
	free(coreType);
	free(coreThreadId);
	free(coreLayer);
	free(core);
}

void sEngine::train(int simulationId_, sTS2* trainTS_, int sampleLen_, int targetLen_, int batchSize_) {
	sampleLen=sampleLen_; targetLen=targetLen_; batchSize=batchSize_;
	for (int i=0; i<2; i++) {
		WTlevel[i]=trainTS_->WTlevel[i];
		WTtype[i]=trainTS_->WTtype[i];
	}
	//===== ONLY WORKING WITH 1 NN CORE ===
	sNNparms* NNcp; safespawn(NNcp, newsname("Core%d_NNparms", 0), defaultdbg, cfg, strBuild("Core%d/Parameters", 0));
	NNcp->setScaleMinMax();

	trainTS_->scale(NNcp->scaleMin[0], NNcp->scaleMax[0]);

	safecall(trainTS_, getDataSet, sampleLen, targetLen, &samplesCnt, &inputCnt, &outputCnt, &sample, &target, &prediction);

	sNN2* NNc; safespawn(NNc, newsname("Core%d_NN", 0), defaultdbg, cfg, "../", inputCnt, outputCnt, NNcp);
	core[0]=NNc;
	core[0]->parms=NNcp;

	core[0]->procArgs->testid=simulationId_;
	core[0]->procArgs->samplesCnt=samplesCnt;
	core[0]->procArgs->batchSize=batchSize;
	core[0]->procArgs->batchCnt=(int)floor(samplesCnt/batchSize);
	core[0]->procArgs->sample=sample;
	core[0]->procArgs->target=target;
	core[0]->procArgs->prediction=prediction;
	core[0]->procArgs->pid=pid;
	core[0]->procArgs->tid=GetCurrentThreadId();

	safecall(core[0], train);

	if (core[0]->procArgs->quitAfterBreak) return;

	if (core[0]->persistor->saveMSEFlag) safecall(core[0]->persistor, saveMSE, core[0]->procArgs->pid, core[0]->procArgs->tid, core[0]->procArgs->mseCnt, core[0]->procArgs->duration, core[0]->procArgs->mseT, core[0]->procArgs->mseV);
	if (core[0]->persistor->saveImageFlag) safecall(core[0], saveImage, core[0]->procArgs->pid, core[0]->procArgs->tid, core[0]->procArgs->mseCnt-1);
	free(core[0]->procArgs->mseT);
	free(core[0]->procArgs->mseV);
	free(core[0]->procArgs->duration);

	trMinIN=(numtype*)malloc(inputCnt*sizeof(numtype));
	trMaxIN=(numtype*)malloc(inputCnt*sizeof(numtype));
	trMinOUT=(numtype*)malloc(outputCnt*sizeof(numtype));
	trMaxOUT=(numtype*)malloc(outputCnt*sizeof(numtype));

	int idx=0;
	for (int d=0; d<trainTS_->dataSourcesCnt[0]; d++) {
		for (int f=0; f<trainTS_->featuresCnt[0][d]; f++) {
			for (int l=0; l<(trainTS_->WTlevel[0]+2); l++) {
				trMinIN[idx]=trainTS_->TRmin[0][d][f][l];
				trMaxIN[idx]=trainTS_->TRmax[0][d][f][l];
				idx++;
			}
		}
	}
	idx=0;
	for (int d=0; d<trainTS_->dataSourcesCnt[1]; d++) {
		for (int f=0; f<trainTS_->featuresCnt[1][d]; f++) {
			for (int l=0; l<(trainTS_->WTlevel[1]+2); l++) {
				trMinOUT[idx]=trainTS_->TRmin[1][d][f][l];
				trMaxOUT[idx]=trainTS_->TRmax[1][d][f][l];
				idx++;
			}
		}
	}

	safecall(persistor, saveEngineInfo, pid, type, sampleLen, targetLen, batchSize, WTlevel[0], WTtype[0]);
//	safecall(persistor, saveEngineData, pid, trainTS_->dataSourcesCnt, trainTS_->featuresCnt, trainTS_->WTlevel, trainTS_->WTtype);
	for (int c=0; c<coresCnt; c++) {
		safecall(persistor, saveEngineCoreInfo, pid, c, 0, core[c]->procArgs->tid, coreType[c]);
		safecall(persistor, saveCoreInfo, pid, core[c]->procArgs->tid, CORE_NN, sampleLen, inputCnt, targetLen, outputCnt, batchSize, trMinIN, trMaxIN, trMinOUT, trMaxOUT);
		safecall(core[c]->persistor, save, persistor, pid, core[c]->procArgs->tid);
		safecall(core[c]->parms, save, persistor, pid, core[c]->procArgs->tid);
	}
	//==== Infer on training set ===
	safecall(this, infer, simulationId_, 0, trainTS_, 0);

	free(trMinIN); free(trMaxIN); free(trMinOUT); free(trMaxOUT);

}
void sEngine::infer(int simulationId_, int seqId_, sTS2* inferTS_, int savedEnginePid_) {
	if (savedEnginePid_>0) {
		int idx=0;
		for (int d=0; d<inferTS_->dataSourcesCnt[0]; d++) {
			for (int f=0; f<inferTS_->featuresCnt[0][d]; f++) {
				for (int l=0; l<(inferTS_->WTlevel[0]+2); l++) {
					inferTS_->TRmin[0][d][f][l]=trMinIN[idx];
					inferTS_->TRmax[0][d][f][l]=trMaxIN[idx];
					idx++;
				}
			}
		}
		idx=0;
		for (int d=0; d<inferTS_->dataSourcesCnt[1]; d++) {
			for (int f=0; f<inferTS_->featuresCnt[1][d]; f++) {
				for (int l=0; l<(inferTS_->WTlevel[1]+2); l++) {
					inferTS_->TRmin[1][d][f][l]=trMinOUT[idx];
					inferTS_->TRmax[1][d][f][l]=trMaxOUT[idx];
					idx++;
				}
			}
		}

		inferTS_->scale(core[0]->parms->scaleMin[0], core[0]->parms->scaleMax[0]);

		//-- re-build core[0]->procArgs from inferTS_
		safecall(inferTS_, getDataSet, sampleLen, targetLen, &samplesCnt, &inputCnt, &outputCnt, &sample, &target, &prediction);

		core[0]->procArgs->testid=simulationId_;
		core[0]->procArgs->samplesCnt=samplesCnt;
		core[0]->procArgs->batchSize=batchSize;
		core[0]->procArgs->batchCnt=(int)floor(samplesCnt/batchSize);
		core[0]->procArgs->sample=sample;
		core[0]->procArgs->target=target;
		core[0]->procArgs->prediction=prediction;
		core[0]->procArgs->pid=pid;
		core[0]->procArgs->tid=GetCurrentThreadId();

	}
	
	core[0]->infer();

	//-- get predictions into ts->prdTRS
	inferTS_->getPrediction(samplesCnt, sampleLen, targetLen, prediction);
	//-- unscale prdTRS into prdTR
	inferTS_->unscale();
	//-- untransform prdTR into prd
	inferTS_->untransform();
	//-- persist (OUTPUT only)
	if (core[0]->persistor->saveRunFlag) {
		for (int d=0; d<inferTS_->dataSourcesCnt[1]; d++) {
			for (int f=0; f<inferTS_->featuresCnt[1][d]; f++) {
				for (int l=0; l<(inferTS_->WTlevel[1]+2); l++) {
					core[0]->persistor->saveRun2(core[0]->procArgs->pid, core[0]->procArgs->tid, core[0]->procArgs->npid, core[0]->procArgs->ntid, seqId_, core[0]->procArgs->mseR, \
						inferTS_->stepsCnt, inferTS_->timestamp, 1, d, f, l, inferTS_->valTRS, inferTS_->prdTRS, inferTS_->valTR, inferTS_->prdTR, inferTS_->val, inferTS_->prd
					);
				}
			}
		}
	}

}

void sEngine::commit() {
	for (int c=0; c<coresCnt; c++) {
		safecall(core[c]->persistor, commit);
	}
}

