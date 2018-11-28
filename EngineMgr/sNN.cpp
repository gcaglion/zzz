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
void sNN::calcErr(numtype* tse_) {
	CEstart=timeGetTime(); CEcnt++;

	//-- sets e, bte; adds squared sum(e) to tse_
	Alg->Vdiff(nodesCnt[outputLevel], &F[levelFirstNode[outputLevel]], 1, u, 1, e);	// e=F[2]-u
	Alg->Vssum(nodesCnt[outputLevel], e, se);										// se=ssum(e) 
	Alg->Vadd(1, tse_, 1, se, 1, tse_);												// tse+=se;

	CEtimeTot+=((DWORD)(timeGetTime()-CEstart));
}
void sNN::calcErrG(sDataSet* ds, numtype* atW) {
	CEstart=timeGetTime(); CEcnt++;

	//-- first, backup W into prevW, then copy atW into W
	if (atW!=nullptr) {
		Alg->Vcopy(weightsCntTotal, W, prevW);
		Alg->Vcopy(weightsCntTotal, atW, W);
	}

	//-- reset TSE once
	Alg->Vinit(1, tse, 0, 0);

	for (int b=0; b<ds->batchCnt; b++) {
		//-- forward pass 1/3 : load samples and targets
		safecallSilent(this, loadSamplesAndTargets, ds, b, false);
		//-- forward pass 2/3 : feed forward across levels
		safecallSilent(this, FF);
		//-- forward pass 3/3 : calc error (sets e[], te, updates tse) for the whole batch
		safecallSilent(this, calcErr, tse);
	}
	
	//-- 3. if a specific W is supplied, restore W
	if (atW!=nullptr) {
		Alg->Vcopy(weightsCntTotal, prevW, W);
	}

	CEtimeTot+=((DWORD)(timeGetTime()-CEstart));
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

	//-- SCGD-specific mallocs
	if (parms->BP_Algo==BP_SCGD) safespawn(scgd, newsname("%s_SCGD_stuff", name->base), defaultdbg, Alg, weightsCntTotal, nodesCnt[outputLevel]);
}
void sNN::destroyWeights() {
	Alg->myFree(W);
	Alg->myFree(prevW);
	Alg->myFree(dW);
	Alg->myFree(dJdW);
}

void sNN::dEdWcalc(numtype* W_, numtype* dEdW_) {
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
			A=&W_[Astart];
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
		C=&dEdW_[Cstart];

		// dJdW(l-1) = edF(l) * F(l-1)
		Alg->MbyM(Ay, Ax, 1, false, A, By, Bx, 1, true, B, C);

	}
}
void sNN::dWcalc(numtype* dEdW_, numtype* dW_) {
	switch (parms->BP_Algo) {
	case BP_STD:
		//-- 1. calc dW = LM*dW - LR*dJdW
		Alg->Vdiff(weightsCntTotal, dW_, parms->LearningMomentum, dEdW_, parms->LearningRate, dW_);
		break;
	case BP_SCGD:break;
	case BP_LM: fail("Not implemented."); break;
	case BP_QING: fail("Not implemented."); break;
	case BP_QUICKPROP: fail("Not implemented."); break;
	case BP_RPROP: fail("Not implemented."); break;
	}

}
void sNN::Wupdate(numtype* W_, numtype* dW_){
	Alg->Vadd(weightsCntTotal, W_, 1, dW_, 1, W_);
}

void sNN::loadSamplesAndTargets(sDataSet* ds, int batchId, bool inferring){	//-- 1. load samples (and targets, if passed) from single batch in dataset onto input layer
	LDstart=timeGetTime(); LDcnt++;

	int L0SampleNodesCnt=ds->sampleLen*ds->selectedFeaturesCnt*ds->batchSamplesCnt;

	//-- load batch samples on L0
	Alg->h2d(&F[(parms->useBias) ? 1 : 0], &ds->sampleBFS[batchId*L0SampleNodesCnt], L0SampleNodesCnt*sizeof(numtype));
	//-- load batch target on output level
	Alg->h2d(&u[0], &ds->targetBFS[batchId*nodesCnt[outputLevel]], nodesCnt[outputLevel]*sizeof(numtype));

	LDtimeTot+=((DWORD)(timeGetTime()-LDstart));

}
void sNN::FF() {
	FFstart=timeGetTime(); FFcnt++;

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
	FFtimeTot+=((DWORD)(timeGetTime()-FFstart));
}

void sNN::ForwardPass(sDataSet* ds, int batchId, bool inferring) {

	//-- forward pass 1/3 : load samples and targets
	safecallSilent(this, loadSamplesAndTargets, ds, batchId, false);
	//-- forward pass 2/3 : feed forward across levels
	safecallSilent(this, FF);
	//-- forward pass 3/3 : calc error (sets e[], te, updates tse) for the whole batch
	safecallSilent(this, calcErr, tse);

	//-- 4. if Inferring, save results for current batch in batchPrediction
	if (inferring) Alg->d2h(&ds->predictionBFS[batchId*nodesCnt[outputLevel]], &F[levelFirstNode[outputLevel]], nodesCnt[outputLevel]*sizeof(numtype));
	
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

	//---- 0.3. Init dW, dJdW
	Alg->Vinit(weightsCntTotal, dW, 0, 0);
	Alg->Vinit(weightsCntTotal, dJdW, 0, 0);

	//-- 0.4. convert samples and targets from SBF to BFS  in training dataset
	trainArgs->ds->setBFS();

	if (parms->BP_Algo==BP_LM||parms->BP_Algo==BP_SCGD) {
		//-- Global BackPropagation Methods (All samples at once, no epochs)
		trainArgs->mseCnt=2;

		//-- get mse at the start of the global process

		//-- 1.0. reset global tse
		Alg->Vinit(1, tse, 0, 0);
		
		//-- calc initial dJdW
		for (int b=0; b<trainArgs->ds->batchCnt; b++) ForwardPass(trainArgs->ds, b, false);

		//-- calc initial Global Error (with current W), and store it in tse
//		safecallSilent(this, calcErrG, trainArgs->ds);

		Alg->d2h(&tse_h, tse, sizeof(numtype));
		numtype mse0 = tse_h/trainArgs->ds->samplesCnt/nodesCnt[outputLevel];
		
		//-- do the actual BackPropagation
		if (parms->BP_Algo==BP_SCGD) {
			BP_scgd(trainArgs->ds);
		} else {
			//BP_lm();
		}

		//-- get mse at the end of the global process
		calcErrG(trainArgs->ds);	//-- sets scgd->gse 
		Alg->d2h(&tse_h, tse, sizeof(numtype));
		numtype mse1 = tse_h/trainArgs->ds->samplesCnt/nodesCnt[outputLevel];

		trainArgs->internalsCnt = scgd->progK;

	} else {
		//-- Local BackPropagation Methods: for every epoch, train all batches with one Forward pass ( loadSamples(b)+FF()+calcErr() ), and one Backward pass (BP + calcdW + W update)
		for (epoch=0; epoch<parms->MaxEpochs; epoch++) {

			//-- timing
			epoch_starttime=timeGetTime();

			//-- 1.0. reset epoch tse
			Alg->Vinit(1, tse, 0, 0);

			//-- 1.1. train one batch at a time
			for (b=0; b<trainSet->batchCnt; b++) {

				//-- forward pass
				ForwardPass(trainSet, b, false);

				//-- backward pass 1/3 : calc dE/dW, and save it into dJdW
				safecallSilent(this, dEdWcalc, W, dJdW);
				//-- backward pass 2/3 : calc dW = LM*dW - LR*dJdW
				safecallSilent(this, dWcalc, dJdW, dW);
				//-- backward pass 2/3 : update W = W + dW for current batch
				safecallSilent(this, Wupdate, W, dW);

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
			if ((parms->StopOnDivergence && epoch>0&&procArgs->mseT[epoch] > procArgs->mseT[epoch-1])) break;
			//-- save weights every <NetSaveFreq> epochs - TO DO!!
			if ((epoch%parms->NetSaveFreq)==0) {}

		}
		trainArgs->mseCnt=epoch-((epoch>parms->MaxEpochs) ? 1 : 0);
	}
	//-- 1. for every epoch, train all batches with one Forward pass ( loadSamples(b)+FF()+calcErr() ), and one Backward pass (BP + calcdW + W update)

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


void sNN::BP_scgd(sDataSet* trainSet_) {

	int k;

	numtype sigma, delta, mu, alpha, beta = 0, b1, b2;
	numtype lambda, lambdau;
	numtype pnorm2;
	numtype e_old=0, e_new=0, comp;
	bool success;
	numtype epsilon = parms->TargetMSE/nodesCnt[outputLevel];

	Alg->Vinit(weightsCntTotal, scgd->TotdW, 0, 0);

	//-- calc initial dE/dW, and save it into dJdW
	safecallSilent(this, dEdWcalc, W, dJdW);

	//-- 1. Choose initial vector w ; p=r=-E'(w)
	Alg->Vcopy(weightsCntTotal, dJdW, scgd->p); Alg->Vscale(weightsCntTotal, scgd->p, -1, scgd->p);
	Alg->Vcopy(weightsCntTotal, scgd->p, scgd->r);

	success = true;
	sigma = (numtype)1e-4;
	lambda = (numtype)1e-6; lambdau = (numtype)0;

	k = 0;
	do {
		Alg->Vnorm(weightsCntTotal, scgd->r, &scgd->rnorm);
		Alg->Vnorm(weightsCntTotal, scgd->p, &scgd->pnorm);
		pnorm2 = pow(scgd->pnorm, 2);

		//-- 2. if success=true Calculate second-order  information (s and delta)
		if (success) {

			//-- non-Hessian approximation
			sigma = sigma/scgd->pnorm;
			//-- get dE0 (dJdW at current W)
			Alg->Vcopy(weightsCntTotal, dJdW, scgd->dE0);
			//-- get dE1 (dJdW at W+sigma*p)
			Alg->Vadd(weightsCntTotal, W, 1, scgd->p, sigma, scgd->newW);
			dEdWcalc(scgd->newW, scgd->dE1);
			//-- calc s
			Alg->Vadd(weightsCntTotal, scgd->dE1, 1, scgd->dE0, -1, scgd->dE);
			Alg->Vscale(weightsCntTotal, scgd->dE, sigma, scgd->s);
			//-- calc delta
			Alg->VdotV(weightsCntTotal, scgd->p, scgd->s, &delta);
		}

		//-- 3. scale s and delta

		//--- 3.1 s=s+(lambda-lambdau)*p
		Alg->Vadd(weightsCntTotal, scgd->s, 1, scgd->p, (lambda-lambdau), scgd->s);
		//--- 3.2 delta=delta+(lambda-lambdau)*|p|^2
		delta += (lambda-lambdau)*pnorm2;

		//-- 4. if delta<=0 (i.e. Hessian is not positive definite) , then make it positive
		if (delta<=0) {
			//-- adjust s
			Alg->Vadd(weightsCntTotal, scgd->s, 1, scgd->p, (lambda-2*delta/pnorm2), scgd->s);
			//-- adjust lambdau
			lambdau = 2*(lambda-delta/pnorm2);
			//-- adjust delta
			delta = -delta+lambda*pnorm2;
			//-- adjust lambda
			lambda = lambdau;
		}

		//-- 5. Calculate step size
		Alg->VdotV(weightsCntTotal, scgd->p, scgd->r, &mu);
		alpha = mu/delta;

		//-- 6. Comparison parameter

		//--- 6.1 calc newW=w+alpha*p , which will also be used in (7)
		Alg->Vadd(weightsCntTotal, W, 1, scgd->p, alpha, scgd->newW);

		if (success) Alg->Vcopy(weightsCntTotal, W, scgd->oldW);
		//--- 6.2 E(w)
		calcErrG(trainSet_, scgd->oldW);
		Alg->d2h(&e_old, tse, sizeof(numtype));
		//--- 6.3 E(w+dw) is calculated by E_at_w()
		calcErrG(trainSet_, scgd->newW);
		Alg->d2h(&e_new, tse, sizeof(numtype));

		//--- 6.4 comp=2*delta*(e_old-e_new)/mu^2
		comp = 2*delta*(e_old-e_new)/pow(mu, 2);

		if (comp>=0) {
			//-- 7. Update weight vector

			//-- W = W + alpha * p
			Alg->Vadd(weightsCntTotal, W, 1, scgd->p, alpha, W);
			//-- TotdW = TotdW + alpha * p
			Alg->Vadd(weightsCntTotal, scgd->TotdW, 1, scgd->p, alpha, scgd->TotdW);
			//-- 7.1 recalc  dJdW
			safecallSilent(this, calcErrG, trainSet_);

			//-- save r, and calc new r
			Alg->Vcopy(weightsCntTotal, scgd->r, scgd->prev_r);
			Alg->Vcopy(weightsCntTotal, scgd->GdJdW, scgd->r); Alg->Vscale(weightsCntTotal, scgd->r, -1, scgd->r);

			//-- reset lambdau
			lambdau = 0; success = true;

			//-- 7a. if k mod N = 0 then restart algorithm, else create new conjugate direction
			if (((k+1)%nodesCntTotal)==0) {
				Alg->Vcopy(weightsCntTotal, scgd->r, scgd->p);
			} else {
				Alg->Vnorm(weightsCntTotal, scgd->r, &b1);
				b1=pow(b1, 2);
				Alg->VdotV(weightsCntTotal, scgd->r, scgd->prev_r, &b2);
				beta = (b1-b2)/mu;
				//-- p = r + beta*p
				Alg->Vadd(weightsCntTotal, scgd->r, 1, scgd->p, beta, scgd->p);
			}
			//-- 7b. if comp>=0.75 reduce scale parameter
			if (comp>=0.75) lambda = lambda/2;

		} else {
			//-- a reduction in error is not possible.
			lambdau = lambda;
			success = false;
		}

		//-- 8. if comp<0.25 then increase scale parameter
		if (comp<0.25) lambda = lambda*4;

		//-- 9. if the steepest descent direction r>epsilon and success=true, then set k=k+1 and go to 2, else terminate and return w as the desired minimum
		Alg->Vnorm(weightsCntTotal, scgd->r, &scgd->rnorm);
		//-- display progress
		//WaitForSingleObject(Mx->ScreenMutex, 10);
		//gotoxy(0, Mx->ScreenPos); printf("\rProcess %6d, Thread %6d, Iteration %6d , success=%s, rnorm=%f , norm_ge=%f", pid, tid, k, (success) ? "TRUE " : "FALSE", scgd->rnorm, norm_ge[t0]);
		gotoxy(0, 0); printf("\rProcess %6d, Thread %6d, Iteration %6d , success=%s, rnorm=%f", pid, tid, k, (success) ? "TRUE " : "FALSE", scgd->rnorm);
		//ReleaseMutex(Mx->ScreenMutex);

		//if (DebugParms->SaveInternals>0) SaveCoreData_SCGD(NNLogs, pid, tid, pEpoch, Mx->sampleid, Mx->BPCount, k, Mx->SCGD_progK, delta, mu, alpha, beta, lambda, lambdau, pnorm, scgd->rnorm, norm_ge[t0], Alg->Vnorm(weightsCntTotal, scgd->dW), comp);

		k++; scgd->progK++;
	} while ((scgd->rnorm>0)&&(k<parms->SCGDmaxK));

	//RestoreW(NN, W, t4);

}
