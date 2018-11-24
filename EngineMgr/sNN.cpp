#include "sNN.h"

sNN::sNN(sCfgObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_, sNNparms* NNparms_) : sCore(sCfgObjParmsVal, layout_, persistor_) {
	parms=NNparms_;

}
sNN::sNN(sCfgObjParmsDef, sCoreLayout* layout_, sNNparms* NNparms_) : sCore(sCfgObjParmsVal, layout_) {
	parms=NNparms_;
	if (parms->useBias) fail("Bias still not working properly. NN creation aborted.");

	//-- init Algebra / CUDA/CUBLAS/CURAND stuff
	safespawn(Alg, newsname("%s_Algebra", name->base), dbg);


	//-- set Common Layout, independent by batchSampleCnt.
	setCommonLayout();	
	//-- weights can be set now, as they are not affected by batchSampleCnt
	createWeights();

	//-- 3. malloc device-based scalar value, to be used by reduction functions (sum, ssum, ...)
	Alg->myMalloc(&se, 1);
	Alg->myMalloc(&tse, 1);

}
sNN::~sNN() {
	Alg->myFree(se);
	Alg->myFree(tse);

	//free(procArgs->mseT); free(procArgs->mseV);

	free(nodesCnt);
	free(levelFirstNode);
	free(ctxStart);

	free(weightsCnt);
	//free(levelFirstWeight);	// 

}

void sNN::setCommonLayout() {
	outputLevel=parms->levelsCnt-1;
	//-- allocate level-specific parameters
	nodesCnt=(int*)malloc(parms->levelsCnt*sizeof(int));
	levelFirstNode=(int*)malloc(parms->levelsCnt*sizeof(int));
	ctxStart=(int*)malloc(parms->levelsCnt*sizeof(int));
	weightsCnt=(int*)malloc((outputLevel)*sizeof(int));
	levelFirstWeight=(int*)malloc((outputLevel)*sizeof(int));
}


void sNN::FF() {
	for (int l=0; l<outputLevel; l++) {
		int Ay=nodesCnt[l+1]/_batchSize;
		int Ax=nodesCnt[l]/_batchSize;
		numtype* A=&W[levelFirstWeight[l]];
		int By=nodesCnt[l]/_batchSize;
		int Bx=_batchSize;
		numtype* B=&F[levelFirstNode[l]];
		numtype* C=&a[levelFirstNode[l+1]];

		//-- actual feed forward ( W10[nc1 X nc0] X F0[nc0 X batchSize] => a1 [nc1 X batchSize] )
		FF0start=timeGetTime(); FF0cnt++;
		safecallSilent(Alg, MbyM, Ay, Ax, 1, false, A, By, Bx, 1, false, B, C);
		FF0timeTot+=((DWORD)(timeGetTime()-FF0start));

		//-- activation sets F[l+1] and dF[l+1]
		FF1start=timeGetTime(); FF1cnt++;
		Activate(l+1);
		FF1timeTot+=((DWORD)(timeGetTime()-FF1start));

		//-- feed back to context neurons
		FF2start=timeGetTime(); FF2cnt++;
		if (parms->useContext) {
			Alg->Vcopy(nodesCnt[l+1], &F[levelFirstNode[l+1]], &F[ctxStart[l]]);
		}
		FF2timeTot+=((DWORD)(timeGetTime()-FF2start));
	}

}
void sNN::Activate(int level) {
	// sets F, dF
	int retf, retd;
	int skipBias=(parms->useBias&&level!=(outputLevel))?1:0;	//-- because bias neuron does not exits in outer layer
	int nc=nodesCnt[level]-skipBias;
	numtype* va=&a[levelFirstNode[level]+skipBias];
	numtype* vF=&F[levelFirstNode[level]+skipBias];
	numtype* vdF=&dF[levelFirstNode[level]+skipBias];

	switch (parms->ActivationFunction[level]) {
	case NN_ACTIVATION_TANH:
		retf=Alg->Tanh(nc, va, vF);
		retd=Alg->dTanh(nc, va, vdF);
		break;
	case NN_ACTIVATION_EXP4:
		retf=Alg->Exp4(nc, va, vF);
		retd=Alg->dExp4(nc, va, vdF);
		break;
	case NN_ACTIVATION_RELU:
		retf=Alg->Relu(nc, va, vF);
		retd=Alg->dRelu(nc, va, vdF);
		break;
	case NN_ACTIVATION_SOFTPLUS:
		retf=Alg->SoftPlus(nc, va, vF);
		retd=Alg->dSoftPlus(nc, va, vdF);
		break;
	default:
		retf=-1;
		break;
	}
	if (!(retf&&retd)) fail("retf=%d ; retd=%d", retf, retd);

}
void sNN::calcErr() {
	//-- sets e, bte; adds squared sum(e) to tse
	Alg->Vdiff(nodesCnt[outputLevel], &F[levelFirstNode[outputLevel]], 1, u, 1, e);	// e=F[2]-u
	Alg->Vssum(nodesCnt[outputLevel], e, se);										// se=ssum(e) 
	Alg->Vadd(1, tse, 1, se, 1, tse);												// tse+=se;
}

void sNN::mallocNeurons() {
	//-- malloc neurons (on either CPU or GPU)
	Alg->myMalloc(&a, nodesCntTotal);
	Alg->myMalloc(&F, nodesCntTotal);
	Alg->myMalloc(&dF, nodesCntTotal);
	Alg->myMalloc(&edF, nodesCntTotal);
	Alg->myMalloc(&e, nodesCnt[outputLevel]);
	Alg->myMalloc(&u, nodesCnt[outputLevel]);
}
void sNN::initNeurons(){
	//--
	Alg->Vinit(nodesCntTotal, F, 0, 0);
	//---- the following are needed by cublas version of MbyM
	Alg->Vinit(nodesCntTotal, a, 0, 0);
	Alg->Vinit(nodesCntTotal, dF, 0, 0);
	Alg->Vinit(nodesCntTotal, edF, 0, 0);

	if (parms->useBias) {
		for (int l=0; l<outputLevel; l++) {
			//-- set every bias node's F=1
			Alg->Vinit(1, &F[levelFirstNode[l]], 1, 0);
		}
	}
}
void sNN::destroyNeurons() {
	Alg->myFree(a);
	Alg->myFree(F);
	Alg->myFree(dF);
	Alg->myFree(edF);
	Alg->myFree(e);
	Alg->myFree(u);
}
void sNN::createWeights() {
	//-- need to set weightsCntTotal, which will not be affected by batchSampleCnt
	setLayout(1);
	//-- malloc weights (on either CPU or GPU)
	Alg->myMalloc(&W, weightsCntTotal);
	Alg->myMalloc(&prevW, weightsCntTotal);
	Alg->myMalloc(&dW, weightsCntTotal);
	Alg->myMalloc(&dJdW, weightsCntTotal);
}
void sNN::destroyWeights() {
	Alg->myFree(W);
	Alg->myFree(prevW);
	Alg->myFree(dW);
	Alg->myFree(dJdW);
}

void sNN::BP_std(){
	int Ay, Ax, Astart, By, Bx, Bstart, Cy, Cx, Cstart;
	numtype* A; numtype* B; numtype* C;

	for (int l = outputLevel; l>0; l--) {
		if (l==(outputLevel)) {
			//-- top level only
			Alg->VbyV2V(nodesCnt[l], e, &dF[levelFirstNode[l]], &edF[levelFirstNode[l]]);	// edF(l) = e * dF(l)
		} else {
			//-- lower levels
			Ay=nodesCnt[l+1]/_batchSize;
			Ax=nodesCnt[l]/_batchSize;
			Astart=levelFirstWeight[l];
			A=&W[Astart];
			By=nodesCnt[l+1]/_batchSize;
			Bx=_batchSize;
			Bstart=levelFirstNode[l+1];
			B=&edF[Bstart];
			Cy=Ax;	// because A gets transposed
			Cx=Bx;
			Cstart=levelFirstNode[l];
			C=&edF[Cstart];

			Alg->MbyM(Ay, Ax, 1, true, A, By, Bx, 1, false, B, C);	// edF(l) = edF(l+1) * WT(l)
			Alg->VbyV2V(nodesCnt[l], &edF[levelFirstNode[l]], &dF[levelFirstNode[l]], &edF[levelFirstNode[l]]);	// edF(l) = edF(l) * dF(l)
		}

		//-- common	
		Ay=nodesCnt[l]/_batchSize;
		Ax=_batchSize;
		Astart=levelFirstNode[l];
		A=&edF[Astart];
		By=nodesCnt[l-1]/_batchSize;
		Bx=_batchSize;
		Bstart=levelFirstNode[l-1];
		B=&F[Bstart];
		Cy=Ay;
		Cx=By;// because B gets transposed
		Cstart=levelFirstWeight[l-1];
		C=&dJdW[Cstart];

		// dJdW(l-1) = edF(l) * F(l-1)
		Alg->MbyM(Ay, Ax, 1, false, A, By, Bx, 1, true, B, C);

	}

}
void sNN::WU_std(){

	//-- 1. calc dW = LM*dW - LR*dJdW
	Alg->Vdiff(weightsCntTotal, dW, parms->LearningMomentum, dJdW, parms->LearningRate, dW);

	//-- 2. update W = W + dW for current batch
	Alg->Vadd(weightsCntTotal, W, 1, dW, 1, W);

}
void sNN::ForwardPass(sDataSet* ds, int batchId, bool inferring) {

	//-- 1. load samples (and targets, if passed) from single batch in dataset onto input layer
	LDstart=timeGetTime(); LDcnt++;

	int L0SampleNodesCnt=ds->sampleLen*ds->selectedFeaturesCnt*ds->batchSamplesCnt;
	//int L0CtxNodesCnt=nodesCnt[0]-L0SampleNodesCnt;
	
	//-- zero context neurons on level 0
	//Alg->Vinit(L0CtxNodesCnt, &F[L0SampleNodesCnt], 0, 0);
	//-- load batch samples on L0
	Alg->h2d(&F[(parms->useBias)?1:0], &ds->sampleBFS[batchId*L0SampleNodesCnt], L0SampleNodesCnt*sizeof(numtype));
	//-- load batch target on output level
	Alg->h2d(&u[0], &ds->targetBFS[batchId*nodesCnt[outputLevel]], nodesCnt[outputLevel]*sizeof(numtype));
	LDtimeTot+=((DWORD)(timeGetTime()-LDstart));

	//-- 2. Feed Forward
	FFstart=timeGetTime(); FFcnt++;	
	FF();
	//safecall(FF());
	FFtimeTot+=((DWORD)(timeGetTime()-FFstart));

	//-- 3. Calc Error (sets e[], te, updates tse) for the whole batch
	CEstart=timeGetTime(); CEcnt++;
	calcErr();
	CEtimeTot+=((DWORD)(timeGetTime()-CEstart));

	//-- 4. if Inferring, save results for current batch in batchPrediction
	if (inferring) Alg->d2h(&ds->predictionBFS[batchId*nodesCnt[outputLevel]], &F[levelFirstNode[outputLevel]], nodesCnt[outputLevel]*sizeof(numtype));
	
}
void sNN::BackwardPass(sDataSet* ds, int batchId, bool updateWeights) {

	//-- 1. BackPropagate, calc dJdW for for current batch
	BPstart=timeGetTime(); BPcnt++;
	BP_std();
	BPtimeTot+=((DWORD)(timeGetTime()-BPstart));

	//-- 2. Weights Update for current batch
	WUstart=timeGetTime(); WUcnt++;
	if (updateWeights) {
		WU_std();
	}
	WUtimeTot+=((DWORD)(timeGetTime()-WUstart));

}
void sNN::showEpochStats(int e, DWORD eStart_) {
	//=======  !!!! CHECK FOR PERFORMANCE DEGRADATION !!!  ========
	char remainingTimeS[TIMER_ELAPSED_FORMAT_LEN];
	
	DWORD epochms=timeGetTime()-eStart_;
	DWORD remainingms=(parms->MaxEpochs-e)*epochms;
	SgetElapsed(remainingms, remainingTimeS);
	//=======  !!!! CHECK FOR PERFORMANCE DEGRADATION !!!  ========

	gotoxy(0, procArgs->screenLine);
	printf("\rTestId %3d, Process %6d, Thread %6d, Epoch %6d/%6d , Training MSE=%1.10f , Validation MSE=%1.10f, duration=%d ms , remaining: %s", testid, pid, tid, e, parms->MaxEpochs, procArgs->mseT[e], procArgs->mseV[e], epochms, remainingTimeS);
}

//-- local implementations of sCore virtual methods
void sNN::setLayout(int batchSamplesCnt_) {
	int l, nl;


	//-- 0.3. set nodesCnt (single sample)
	nodesCnt[0] = layout->inputCnt*batchSamplesCnt_;
	nodesCnt[outputLevel] = layout->outputCnt*batchSamplesCnt_;
	for (nl = 0; nl<(parms->levelsCnt-2); nl++) nodesCnt[nl+1] = (int)floor(nodesCnt[nl]*parms->levelRatio[nl]);

	//-- add context neurons
	if (parms->useContext) {
		for (nl = outputLevel; nl>0; nl--) nodesCnt[nl-1] += nodesCnt[nl];
	}
	//-- add one bias neurons for each layer, except output layer
	if (parms->useBias) {
		for (nl = 0; nl<(outputLevel); nl++) nodesCnt[nl] += 1;
	}

	//-- 0.2. calc nodesCntTotal
	nodesCntTotal=0;
	for (l=0; l<parms->levelsCnt; l++) nodesCntTotal+=nodesCnt[l];

	//-- 0.3. weights count
	weightsCntTotal=0;
	for (l=0; l<(outputLevel); l++) {
		weightsCnt[l]=nodesCnt[l]/batchSamplesCnt_*nodesCnt[l+1]/batchSamplesCnt_;
		weightsCntTotal+=weightsCnt[l];
	}

	//-- 0.4. set first node and first weight for each layer
	for (l=0; l<parms->levelsCnt; l++) {
		levelFirstNode[l]=0;
		levelFirstWeight[l]=0;
		for (int ll=0; ll<l; ll++) {
			levelFirstNode[l]+=nodesCnt[ll];
			levelFirstWeight[l]+=weightsCnt[ll];
		}
	}

	//-- ctxStart[] can only be defined after levelFirstNode has been defined.
	if (parms->useContext) {
		for (nl=0; nl<(outputLevel); nl++) ctxStart[nl]=levelFirstNode[nl+1]-nodesCnt[nl+1]+((parms->useBias) ? 1 : 0);
	}

}
void sNN::mallocLayout() {
	//-- malloc + init neurons
	safecall(this, mallocNeurons);
	safecall(this, initNeurons);
}
void sNN::train(sCoreProcArgs* trainArgs) {
	int l;
	DWORD epoch_starttime;
	DWORD training_starttime=timeGetTime();
	int epoch, b;
	numtype tse_h;	// total squared error copied on host at the end of each eopch

	//-- extract training arguments from trainArgs into local variables
	sDataSet* trainSet = trainArgs->ds;
	pid=trainArgs->pid;
	tid=trainArgs->tid;
	testid=trainArgs->testid;

	//-- set private _batchCnt and _batchSize for the network from dataset
	_batchCnt=trainSet->batchCnt;
	_batchSize=trainSet->batchSamplesCnt;
	//-- set Layout. This should not change weightsCnt[] at all, just nodesCnt[]
	setLayout(_batchSize);

	//-- 0. malloc + init neurons
	mallocNeurons();
	initNeurons();

	//-- malloc mse[maxepochs], always host-side. We need to free them, first (see issue when running without training...)
	free(trainArgs->mseT); trainArgs->mseT=(numtype*)malloc(parms->MaxEpochs*sizeof(numtype));
	free(trainArgs->mseV); trainArgs->mseV=(numtype*)malloc(parms->MaxEpochs*sizeof(numtype));

	//---- 0.2. Init W
	for (l=0; l<(outputLevel); l++) Alg->VinitRnd(weightsCnt[l], &W[levelFirstWeight[l]], -1/sqrtf((numtype)nodesCnt[l]), 1/sqrtf((numtype)nodesCnt[l]), Alg->cuRandH);
	//dumpArray(weightsCntTotal, &W[0], "C:/temp/referenceW/initW.txt");
	//loadArray(weightsCntTotal, &W[0], "C:/temp/referenceW/initW.txt");

	//---- 0.3. Init dW, dJdW
	Alg->Vinit(weightsCntTotal, dW, 0, 0);
	Alg->Vinit(weightsCntTotal, dJdW, 0, 0);

	//-- 0.4. convert samples and targets from SBF to BFS  in training dataset
	trainArgs->ds->setBFS();

	//-- 1. for every epoch, train all batches with one Forward pass ( loadSamples(b)+FF()+calcErr() ), and one Backward pass (BP + calcdW + W update)
	for (epoch=0; epoch<parms->MaxEpochs; epoch++) {

		//-- timing
		epoch_starttime=timeGetTime();

		//-- 1.0. reset epoch tse
		Alg->Vinit(1, tse, 0, 0);

		//-- 1.1. train one batch at a time
		for (b=0; b<trainSet->batchCnt; b++) {

			//-- forward pass, with targets
			safecallSilent(this, ForwardPass, trainSet, b, false);

			//-- backward pass, with weights update
			safecallSilent(this, BackwardPass, trainSet, b, true);

		}

		//-- 1.2. calc epoch MSE (for ALL batches), and check criteria for terminating training (targetMSE, Divergence)
		Alg->d2h(&tse_h, tse, sizeof(numtype));
		procArgs->mseT[epoch]=tse_h/nodesCnt[outputLevel]/_batchCnt;
		procArgs->mseV[epoch]=0;	// TO DO !
		//-- 1.3. show epoch info
		showEpochStats(epoch, epoch_starttime);
		//-- break if TargetMSE is reached
		if (procArgs->mseT[epoch]<parms->TargetMSE) break;
		//-- break on divergence
		if ((parms->StopOnDivergence && epoch>0 && procArgs->mseT[epoch] > procArgs->mseT[epoch-1])) break;
		//-- save weights every <NetSaveFreq> epochs - TO DO!!
		if ((epoch%parms->NetSaveFreq)==0) {}

	}
	trainArgs->mseCnt=epoch-((epoch>parms->MaxEpochs)?1:0);

	//-- 2. test run. need this to make sure all batches pass through the net with the latest weights, and training targets
	TRstart=timeGetTime(); TRcnt++;

	Alg->Vinit(1, tse, 0, 0);	
	for (b=0; b<trainSet->batchCnt; b++) ForwardPass(trainSet, b, false);
	Alg->d2h(&tse_h, tse, sizeof(numtype));
	procArgs->mseT[trainArgs->mseCnt-1]=tse_h/nodesCnt[outputLevel]/_batchCnt;
	showEpochStats(trainArgs->mseCnt-1, epoch_starttime);
	
	Alg->Vinit(1, tse, 0, 0);
	for (b=0; b<trainSet->batchCnt; b++) ForwardPass(trainSet, b, false);
	Alg->d2h(&tse_h, tse, sizeof(numtype));
	procArgs->mseT[trainArgs->mseCnt-1]=tse_h/nodesCnt[outputLevel]/_batchCnt;
	showEpochStats(trainArgs->mseCnt-1, epoch_starttime);

	//for (int b=0; b<procArgs->ds->batchCnt; b++) procArgs->ds->BFS2SBF(b, procArgs->ds->predictionLen, procArgs->ds->predictionBFS, procArgs->ds->predictionSBF);
	//dumpArrayH((procArgs->ds->samplesCnt*procArgs->ds->predictionLen*procArgs->ds->selectedFeaturesCnt), procArgs->ds->predictionSBF, "C:/temp/TRAINpredictionSBF.csv");
	//dumpArrayH((procArgs->ds->samplesCnt*procArgs->ds->predictionLen*procArgs->ds->selectedFeaturesCnt), procArgs->ds->targetSBF, "C:/temp/TRAINtargetSBF.csv");


	TRtimeTot+=((DWORD)(timeGetTime()-TRstart));

/*	float elapsed_tot=(float)timeGetTime()-(float)training_starttime;
	float elapsed_avg=elapsed_tot/trainArgs->mseCnt;
	printf("\nTraining complete. Elapsed time: %0.1f seconds. Epoch average=%0.0f ms.\n", (elapsed_tot/(float)1000), elapsed_avg);
	LDtimeAvg=(float)LDtimeTot/LDcnt; printf("LD count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", LDcnt, (LDtimeTot/(float)1000), LDtimeAvg);
	FFtimeAvg=(float)FFtimeTot/FFcnt; printf("FF count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", FFcnt, (FFtimeTot/(float)1000), FFtimeAvg);
	FF0timeAvg=(float)FF0timeTot/FF0cnt; printf("FF0 count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", FF0cnt, (FF0timeTot/(float)1000), FF0timeAvg);
	FF1timeAvg=(float)FF1timeTot/FF1cnt; printf("FF1 count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", FF1cnt, (FF1timeTot/(float)1000), FF1timeAvg);
	//FF1atimeAvg=(float)FF1atimeTot/FF1acnt; printf("FF1a count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", FF1acnt, (FF1atimeTot/(float)1000), FF1atimeAvg);
	//FF1btimeAvg=(float)FF1btimeTot/FF1bcnt; printf("FF1b count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", FF1bcnt, (FF1btimeTot/(float)1000), FF1btimeAvg);
	//FF2timeAvg=(float)FF2timeTot/FF2cnt; printf("FF2 count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", FF2cnt, (FF2timeTot/(float)1000), FF2timeAvg);
	CEtimeAvg=(float)CEtimeTot/CEcnt; printf("CE count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", CEcnt, (CEtimeTot/(float)1000), CEtimeAvg);
	//VDtimeAvg=(float)VDtimeTot/VDcnt; printf("VD count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", VDcnt, (VDtimeTot/(float)1000), VDtimeAvg);
	//VStimeAvg=(float)VStimeTot/VScnt; printf("VS count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", VScnt, (VStimeTot/(float)1000), VStimeAvg);
	BPtimeAvg=(float)BPtimeTot/LDcnt; printf("BP count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", BPcnt, (BPtimeTot/(float)1000), BPtimeAvg);
	TRtimeAvg=(float)TRtimeTot/LDcnt; printf("TR count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", TRcnt, (TRtimeTot/(float)1000), TRtimeAvg);
*/

	//-- feee neurons()
	destroyNeurons();

}
void sNN::infer(sCoreProcArgs* inferArgs) {

	numtype tse_h;	// total squared error copied on host at the end of infer
	DWORD inferStartTime=timeGetTime();

	//-- extract infering arguments from inferArgs into local variables
	sDataSet* inferSet = inferArgs->ds;
	pid=inferArgs->pid;
	tid=inferArgs->tid;
	testid=inferArgs->testid;

	//-- set private _batchCnt and _batchSize for the network from dataset
	_batchCnt=inferSet->batchCnt;
	_batchSize=inferSet->batchSamplesCnt;
	//-- set Layout. This should not change weightsCnt[] at all, just nodesCnt[]
	setLayout(_batchSize);

	//-- 0. malloc + init neurons
	mallocNeurons();
	initNeurons();

	//-- 0.3. W[] should have already been loaded

	//-- 0.4. convert samples and targets from SBF to BFS  in inference dataset
	inferArgs->ds->setBFS();

	//-- 1. infer all batches with one Forward pass ( loadSamples(b)+FF()+calcErr() ). No backward pass, obviously.

	//-- timing
	inferStartTime=timeGetTime();

	Alg->Vinit(1, tse, 0, 0);
	for (int b=0; b<inferSet->batchCnt; b++) ForwardPass(inferSet, b, true);
	Alg->d2h(&tse_h, tse, sizeof(numtype));
	procArgs->mseR=tse_h/nodesCnt[outputLevel]/_batchCnt;

	Alg->Vinit(1, tse, 0, 0);
	for (int b=0; b<inferSet->batchCnt; b++) ForwardPass(inferSet, b, true);
	Alg->d2h(&tse_h, tse, sizeof(numtype));
	procArgs->mseR=tse_h/nodesCnt[outputLevel]/_batchCnt;

	//-- 0.4. convert samples and targets back from BFS to SBF in inference dataset
	inferArgs->ds->setSBF();

	//=======================================
	//numtype _se;
	//numtype _tse=0;
	//numtype _mse;
	//for (int i=0; i<(procArgs->ds->samplesCnt*procArgs->ds->predictionLen*procArgs->ds->selectedFeaturesCnt); i++) {
	//	_se=pow((procArgs->ds->predictionSBF[i]-procArgs->ds->targetSBF[i]), 2);
	//	_tse+=_se;
	//}
	//dumpArrayH((procArgs->ds->samplesCnt*procArgs->ds->predictionLen*procArgs->ds->selectedFeaturesCnt), procArgs->ds->predictionSBF, "C:/temp/INFERpredictionSBF.csv");
	//dumpArrayH((procArgs->ds->samplesCnt*procArgs->ds->predictionLen*procArgs->ds->selectedFeaturesCnt), procArgs->ds->targetSBF, "C:/temp/INFERtargetSBF.csv");

	//_mse=_tse/(procArgs->ds->samplesCnt*procArgs->ds->predictionLen*procArgs->ds->selectedFeaturesCnt);
	//=======================================

	//-- feee neurons()
	destroyNeurons();

}

//-- local implementations of sCore virtual methods
void sNN::saveImage(int pid, int tid, int epoch) {

	//-- malloc clocal copy of W
	numtype* hW = (numtype*)malloc(weightsCntTotal*sizeof(numtype));
	//-- get data if it's on the GPU
	Alg->d2h(hW, W, weightsCntTotal*sizeof(numtype));
	//-- call persistor to save hW
	safecall(persistor, saveCoreNNImage, pid, tid, epoch, weightsCntTotal, hW);
	//-- free local copy
	free(hW);

}
void sNN::loadImage(int pid, int tid, int epoch) {

	//-- malloc clocal copy of W
	numtype* hW = (numtype*)malloc(weightsCntTotal*sizeof(numtype));
	//-- call persistor to load hW
	safecall(persistor, loadCoreNNImage, pid, tid, epoch, weightsCntTotal, hW);
	//-- load data if it's on the GPU
	Alg->h2d(W, hW, weightsCntTotal*sizeof(numtype));
	//-- free local copy
	free(hW);

}

