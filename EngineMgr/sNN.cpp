#include "sNN.h"

void sNN::sNNcommon(sNNparms* NNparms_) {
	parms=NNparms_;
	if (parms->useBias) fail("Bias still not working properly. NN creation aborted.");

	//-- set Common Layout, independent by batchSampleCnt.
	setCommonLayout();
	//-- weights can be set now, as they are not affected by batchSampleCnt
	createWeights();
}
sNN::sNN(sObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sCoreLogger* persistor_, sNNparms* NNparms_) : sCore(sObjParmsVal, nullptr, nullptr, Alg_, layout_, persistor_) {
	sNNcommon(NNparms_);
}
sNN::sNN(sCfgObjParmsDef, sAlgebra* Alg_, sCoreLayout* layout_, sNNparms* NNparms_) : sCore(sCfgObjParmsVal, Alg_, layout_) {
	sNNcommon(NNparms_);
}
sNN::~sNN() {

	free(nodesCnt);
	free(levelFirstNode);
	free(ctxStart);

	free(weightsCnt);

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

void sNN::Ecalc() {
	//-- sets e, bte; adds squared sum(e) to tse
	Alg->Vadd(nodesCnt[outputLevel], &F[levelFirstNode[outputLevel]], 1, u, -1, e);	// e=F[2]-u
	Alg->Vssum(nodesCnt[outputLevel], e, se);										// se=ssum(e) 
	Alg->Vadd(1, tse, 1, se, 1, tse);												// tse+=se;
}
void sNN::dEcalc() {
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
void sNN::EcalcG(sDataSet* ds, numtype* inW, numtype* outE) {

	//-- sets outE

	//-- backup current W to bkpW, then set it to inW
	Alg->Vcopy(weightsCntTotal, W, scgd->bkpW);
	Alg->Vcopy(weightsCntTotal, inW, W);

	//-- zero tse, outE
	Alg->Vinit(1, tse, 0, 0);
	(*outE)=0;
	//-- sum tse for every batch into outE
	for (int b=0; b<_batchCnt; b++) {

		//-- load batch input and output
		loadBatchData(ds, b);
		//-- forward pass 
		safecallSilent(this, FF);
		//-- Calc Error (sets e[], te, updates tse) for the whole batch
		safecallSilent(this, Ecalc);

		Alg->d2h(&tse_h, tse, 1*sizeof(numtype), false);
		//-- increment global outE = outE + tse
		(*outE)+=tse_h;

	}

	//-- restore W
	Alg->Vcopy(weightsCntTotal, scgd->bkpW, W);


}
void sNN::dEcalcG(sDataSet* ds, numtype* inW, numtype* outdE) {

	//-- sets outdE


	//-- backup current W to bkpW, then set it to inW
	Alg->Vcopy(weightsCntTotal, W, scgd->bkpW);
	Alg->Vcopy(weightsCntTotal, inW, W);

	//-- zero tse
	Alg->Vinit(1, tse, 0, 0);
	//-- zero outdE
	Alg->Vinit(weightsCntTotal, outdE, 0, 0);

	//-- sum dJdW for every batch into outdE
	for (int b=0; b<_batchCnt; b++) {

		//-- load batch input and output
		loadBatchData(ds, b);
		//-- forward pass 
		safecallSilent(this, FF);
		//-- Calc Error (sets e[], te, updates tse) for the whole batch
		safecallSilent(this, Ecalc);

		//-- set dJdW for current batch
		dEcalc();
		//-- increment outdE = outdE + dJdW
		Alg->Vadd(weightsCntTotal, outdE, 1, dJdW, 1, outdE);

	}

	//-- restore W
	Alg->Vcopy(weightsCntTotal, scgd->bkpW, W);

}

void sNN::mallocNeurons() {
	//-- malloc neurons (on either CPU or GPU)
	Alg->myMalloc(&a, nodesCntTotal);
	Alg->myMalloc(&F, nodesCntTotal);
	Alg->myMalloc(&dF, nodesCntTotal);
	Alg->myMalloc(&edF, nodesCntTotal);
	Alg->myMalloc(&e, nodesCnt[outputLevel]);
	Alg->myMalloc(&u, nodesCnt[outputLevel]);
	Alg->myMalloc(&tse, 1);
	Alg->myMalloc(&se, 1);
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
	Alg->myFree(se);
	Alg->myFree(tse);
}
void sNN::createWeights() {
	//-- need to set weightsCntTotal, which will not be affected by batchSampleCnt
	setLayout(1);
	//-- malloc weights (on either CPU or GPU)
	Alg->myMalloc(&W, weightsCntTotal);
	Alg->myMalloc(&prevW, weightsCntTotal);
	Alg->myMalloc(&dW, weightsCntTotal);
	Alg->myMalloc(&dJdW, weightsCntTotal);
	//-- scgd stuff
	if (parms->BP_Algo==BP_SCGD) scgd = new sSCGD(this, newsname("%s_scgd", name->base), defaultdbg, GUIreporter, Alg, weightsCntTotal, nodesCnt[outputLevel], parms->SCGDmaxK);
}
void sNN::destroyWeights() {
	Alg->myFree(W);
	Alg->myFree(prevW);
	Alg->myFree(dW);
	Alg->myFree(dJdW);
}

void sNN::BP_std(){
	dEcalc();
}
void sNN::WU_std(){

	//-- 1. calc dW = LM*dW - LR*dJdW
	Alg->Vadd(weightsCntTotal, dW, parms->LearningMomentum, dJdW, -parms->LearningRate, dW);

	//-- 2. update W = W + dW for current batch
	Alg->Vadd(weightsCntTotal, W, 1, dW, 1, W);

}

void sNN::loadBatchData(sDataSet* ds, int b) {
	//-- set number of L0 neurons to load
	int L0SampleNodesCnt=ds->shape->sampleLen*ds->shape->featuresCnt*ds->batchSamplesCnt;
	//-- load batch samples on L0
	Alg->d2d(&F[(parms->useBias) ? 1 : 0], &sample_d[b*L0SampleNodesCnt], L0SampleNodesCnt*sizeof(numtype));
	//-- load batch target on output level
	Alg->d2d(&u[0], &target_d[b*nodesCnt[outputLevel]], nodesCnt[outputLevel]*sizeof(numtype));
}
void sNN::ForwardPass(sDataSet* ds, int batchId, bool inferring) {

	//-- 1. load samples (and targets, if passed) from single batch in dataset onto input layer
	LDstart=timeGetTime(); LDcnt++;
	safecallSilent(this, loadBatchData, ds, batchId);
	LDtimeTot+=((DWORD)(timeGetTime()-LDstart));

	//-- 2. Feed Forward
	FFstart=timeGetTime(); FFcnt++;
	safecallSilent(this, FF);
	FFtimeTot+=((DWORD)(timeGetTime()-FFstart));

	//-- 3. Calc Error (sets e[], te, updates tse) for the whole batch. Only if we have targets
	if (ds->hasTargets) {
		CEstart=timeGetTime(); CEcnt++;
		safecallSilent(this, Ecalc);
		CEtimeTot+=((DWORD)(timeGetTime()-CEstart));
	}
	//-- 4. if Inferring, save results for current batch in batchPrediction
	if (inferring) Alg->d2h(&ds->predictionBFS[batchId*nodesCnt[outputLevel]], &F[levelFirstNode[outputLevel]], nodesCnt[outputLevel]*sizeof(numtype));

}
void sNN::BackwardPass(sDataSet* ds, int batchId, bool updateWeights) {


	switch (parms->BP_Algo) {
	case BP_STD:
		//-- 1. BackPropagate, calc dJdW for for current batch
		BPstart=timeGetTime(); BPcnt++;
		BP_std();
		BPtimeTot+=((DWORD)(timeGetTime()-BPstart));
		//-- 2. Weights Update for current batch
		WUstart=timeGetTime(); WUcnt++;
		if (updateWeights) WU_std();
		WUtimeTot+=((DWORD)(timeGetTime()-WUstart));
		break;
	case BP_SCGD:
		break;
	default:
		fail("Backpropagation method not implemented: %d", parms->BP_Algo);
	}



}
void sNN::showEpochStats(int e, DWORD eStart_) {
	//=======  !!!! CHECK FOR PERFORMANCE DEGRADATION !!!  ========
	char remainingTimeS[TIMER_ELAPSED_FORMAT_LEN];

	procArgs->duration[e]=timeGetTime()-eStart_;
	DWORD remainingms=(parms->MaxEpochs-e)*procArgs->duration[e];
	SgetElapsed(remainingms, remainingTimeS);
	//=======  !!!! CHECK FOR PERFORMANCE DEGRADATION !!!  ========
	sprintf_s(dbg->msg, DBG_MSG_MAXLEN, "\rTestId %3d, Process %6d, Thread %6d, Epoch %6d/%6d , Training MSE=%1.10f , Validation MSE=%1.10f, duration=%d ms , remaining: %s", testid, pid, tid, e, parms->MaxEpochs, procArgs->mseT[e], procArgs->mseV[e], procArgs->duration[e], remainingTimeS);

	if (dbg->dbgtoscreen) {
		if (GUIreporter!=nullptr) {
			(*GUIreporter)(20, dbg->msg);
		} else {
			gotoxy(0, procArgs->screenLine);
			printf(dbg->msg);
		}
	}

}
void sNN::showEpochStatsG(int e, DWORD eStart_, bool success_, numtype Gtse_) {
	//=======  !!!! CHECK FOR PERFORMANCE DEGRADATION !!!  ========
	char remainingTimeS[TIMER_ELAPSED_FORMAT_LEN];

	procArgs->duration[e]=timeGetTime()-eStart_;
	DWORD remainingms=(parms->SCGDmaxK-e)*procArgs->duration[e];
	SgetElapsed(remainingms, remainingTimeS);
	procArgs->mseT[e]=Gtse_/nodesCnt[outputLevel]/_batchCnt;

	//=======  !!!! CHECK FOR PERFORMANCE DEGRADATION !!!  ========
	sprintf_s(dbg->msg, DBG_MSG_MAXLEN, "\rTestId %3d, Process %6d, Thread %6d, Iteration %6d/%6d , GlobalMSE=%1.10f , success=%s , duration=%d ms , remaining: %s", testid, pid, tid, e, parms->SCGDmaxK, procArgs->mseT[e], (success_) ? "TRUE " : "FALSE", procArgs->duration[e], remainingTimeS);

	if (dbg->dbgtoscreen) {
		if (GUIreporter!=nullptr) {
			(*GUIreporter)(20, dbg->msg);
		} else {
			gotoxy(0, procArgs->screenLine);
			printf(dbg->msg);
		}
	}

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
	bool hasInverted=false, hasDiverged=false, hasMinimized=false;

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
	trainArgs->duration=(int*)malloc(parms->MaxEpochs*sizeof(int));
	trainArgs->mseT=(numtype*)malloc(parms->MaxEpochs*sizeof(numtype));
	trainArgs->mseV=(numtype*)malloc(parms->MaxEpochs*sizeof(numtype));

	//---- 0.2. Init W
	for (l=0; l<(outputLevel); l++) Alg->VinitRnd(weightsCnt[l], &W[levelFirstWeight[l]], -1/sqrtf((numtype)nodesCnt[l]), 1/sqrtf((numtype)nodesCnt[l]), Alg->cuRandH);
	//dumpArray(weightsCntTotal, W, "C:/temp/initW.txt");
	//loadArray(weightsCntTotal, W, "initW.txt");

	//---- 0.3. Init dW, dJdW
	Alg->Vinit(weightsCntTotal, dW, 0, 0);
	Alg->Vinit(weightsCntTotal, dJdW, 0, 0);

	//-- 0.4. convert samples and targets from SBF to BFS  in training dataset
	trainArgs->ds->setBFS();

	//-- pre-load the whole dataset (samples+targets) on GPU !
	safecall(this, loadWholeDataSet);

	//-- 1. for every epoch, train all batches with one Forward pass ( loadSamples(b)+FF()+calcErr() ), and one Backward pass (BP + calcdW + W update)
	if (parms->BP_Algo==BP_SCGD) {

		//-- main algorithm
		trainArgs->mseCnt=trainSCGD(trainArgs);

	} else {
		for (epoch=0; epoch<parms->MaxEpochs; epoch++) {

			//-- timing
			epoch_starttime=timeGetTime();

			//-- 1.0. reset epoch tse
			Alg->Vinit(1, tse, 0, 0);

			//-- save prev W
			Alg->d2d(prevW, W, weightsCntTotal*sizeof(numtype));

			//-- 1.1. train one batch at a time
			for (b=0; b<trainSet->batchCnt; b++) {

				//-- forward pass, with targets
				safecallSilent(this, ForwardPass, trainSet, b, false);

				//-- backward pass, with weights update
				safecallSilent(this, BackwardPass, trainSet, b, true);

			}

			//-- 1.2. calc epoch MSE (for ALL batches), and check criteria for terminating training (targetMSE, Divergence)
			Alg->d2h(&tse_h, tse, 1*sizeof(numtype), false);
			procArgs->mseT[epoch]=tse_h/nodesCnt[outputLevel]/_batchCnt;
			procArgs->mseV[epoch]=0;	// TO DO !
			//-- 1.3. show epoch info
			showEpochStats(epoch, epoch_starttime);
			//-- break if TargetMSE is reached
			if (procArgs->mseT[epoch]<parms->TargetMSE) {
				hasMinimized=true; 
				break;
			}
			//-- break on inversion
			if ((parms->StopOnInversion && epoch>0&&procArgs->mseT[epoch]>procArgs->mseT[epoch-1])) {
				hasInverted=true;
				break;
			}
			//-- save weights every <NetSaveFreq> epochs - TO DO!!
			if ((epoch%parms->NetSaveFreq)==0) {}

		}
		procArgs->mseCnt=epoch;

		//-- on inversion, restore W from previous epoch
		if (hasInverted) {
			Alg->d2d(W, prevW, weightsCntTotal*sizeof(numtype));
			showEpochStats(procArgs->mseCnt-1, epoch_starttime);
		}

		float elapsed_tot=(float)timeGetTime()-(float)training_starttime;
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
		//	TRtimeAvg=(float)TRtimeTot/LDcnt; printf("TR count=%d ; time-tot=%0.1f s. time-avg=%0.0f ms.\n", TRcnt, (TRtimeTot/(float)1000), TRtimeAvg);
	}

	//-- feee neurons()
	//destroyNeurons();

}
void sNN::infer(sCoreProcArgs* inferArgs) {

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

	if (inferArgs->loadImage) {
		loadImage(inferArgs->npid, inferArgs->ntid, -1);
	}
	//initNeurons();

	//-- 0.4. convert samples and targets from SBF to BFS  in inference dataset
	inferArgs->ds->setBFS();

	//-- pre-load the whole dataset (samples+targets) on GPU !
	safecall(this, loadWholeDataSet);

	//-- 1. infer all batches with one Forward pass ( loadSamples(b)+FF()+calcErr() ). No backward pass, obviously.

	//-- timing
	inferStartTime=timeGetTime();

	Alg->Vinit(1, tse, 0, 0);
	for (int b=0; b<_batchCnt; b++) ForwardPass(inferSet, b, true);
	Alg->d2h(&tse_h, tse, 1*sizeof(numtype), false);
	procArgs->mseR=tse_h/nodesCnt[outputLevel]/_batchCnt;

	Alg->Vinit(1, tse, 0, 0);
	for (int b=0; b<_batchCnt; b++) ForwardPass(inferSet, b, true);
	Alg->d2h(&tse_h, tse, 1*sizeof(numtype), false);
	procArgs->mseR=tse_h/nodesCnt[outputLevel]/_batchCnt;

	//-- 0.4. convert samples and targets back from BFS to SBF in inference dataset
	inferArgs->ds->setSBF();

	//-- feee neurons()
	destroyNeurons();

}

//-- local implementations of sCore virtual methods
void sNN::saveImage(int pid, int tid, int epoch) {

	//-- malloc clocal copy of W,F
	numtype* hW = (numtype*)malloc(weightsCntTotal*sizeof(numtype));
	numtype* hF = (numtype*)malloc(nodesCntTotal*sizeof(numtype));
	//-- get data if it's on the GPU
	Alg->d2h(hW, W, weightsCntTotal*sizeof(numtype));
	Alg->d2h(hF, F, nodesCntTotal*sizeof(numtype));
	//-- call persistor to save hW
	safecall(persistor, saveCoreNNImage, pid, tid, epoch, weightsCntTotal, hW, nodesCntTotal, hF);
	//-- free local copy
	free(hW);
	free(hF);
}
void sNN::loadImage(int pid, int tid, int epoch) {

	//-- malloc clocal copy of W
	numtype* hW = (numtype*)malloc(weightsCntTotal*sizeof(numtype));
	numtype* hF = (numtype*)malloc(nodesCntTotal*sizeof(numtype));
	//-- call persistor to load hW
	safecall(persistor, loadCoreNNImage, pid, tid, epoch, weightsCntTotal, hW, nodesCntTotal, hF);
	//-- load data if it's on the GPU
	Alg->h2d(W, hW, weightsCntTotal*sizeof(numtype));
	Alg->h2d(F, hF, nodesCntTotal*sizeof(numtype));
	//-- free local copy
	free(hW);
	free(hF);
}

int sNN::trainSCGD(sCoreProcArgs* procArgs) {

	bool success;
	numtype pnorm, rnorm;
	numtype sigma = (numtype)1e-4;
	numtype lambda = (numtype)1e-6; numtype lambdau = (numtype)0;
	numtype delta;
	numtype mu;
	numtype alpha, beta=0;
	numtype Gtse_old, Gtse_new, comp;
	numtype rdotprevr;
	numtype newWnorm;	// ====REMOVE===
	numtype Wnorm;		// ====REMOVE===
	numtype dEnorm;		// ====REMOVE===
	numtype snorm;		// ====REMOVE===
	numtype dE0norm;		// ====REMOVE===
	numtype dE1norm;		// ====REMOVE===
							//-- timing
	DWORD kstart;

	Alg->Vnorm(weightsCntTotal, W, &Wnorm);
	//-- 1.1 calc GdJwd
	dEcalcG(procArgs->ds, W, scgd->GdJdW);
	//-- 1.2 p=-GdJdW
	Alg->Vscale(weightsCntTotal, scgd->GdJdW, -1, scgd->p);
	//-- 1.3 r=p
	Alg->Vcopy(weightsCntTotal, scgd->p, scgd->r);
	//-- 1.4 success=true
	success=true;

	int k=0;

	do {
		kstart=timeGetTime();
		//-- calc pnorm
		Alg->Vnorm(weightsCntTotal, scgd->p, &pnorm);

		//-- 2.	if success=true,
		if (success) {

			//-- sigma=sigma/pnorm
			sigma/=pnorm;
			//-- newW=w+sigma*p
			Alg->Vadd(weightsCntTotal, scgd->p, sigma, W, 1, scgd->newW); Alg->Vnorm(weightsCntTotal, scgd->newW, &newWnorm);
			//-- dE0=dEcalcG(W)
			dEcalcG(procArgs->ds, W, scgd->dE0); Alg->Vnorm(weightsCntTotal, scgd->dE0, &dE0norm);
			//-- dE1=dEcalcG(newW)
			dEcalcG(procArgs->ds, scgd->newW, scgd->dE1); Alg->Vnorm(weightsCntTotal, scgd->dE1, &dE1norm);
			//-- dE=dE1-dE0
			Alg->Vadd(weightsCntTotal, scgd->dE1, 1, scgd->dE0, -1, scgd->dE); Alg->Vnorm(weightsCntTotal, scgd->dE, &dEnorm);
			//-- s=(dE)/sigma
			Alg->Vscale(weightsCntTotal, scgd->dE, 1/sigma, scgd->s); Alg->Vnorm(weightsCntTotal, scgd->s, &snorm);
			//-- delta=VdotV(p*s)
			Alg->VdotV(weightsCntTotal, scgd->p, scgd->s, &delta);
		}

		//-- 3.	

		//-- s=s+(lambda-lambdau)*p
		Alg->Vadd(weightsCntTotal, scgd->s, 1, scgd->p, (lambda-lambdau), scgd->s); Alg->Vnorm(weightsCntTotal, scgd->s, &snorm);
		//-- delta=delta+(lambda-lambdau)*pnorm2
		delta+=((lambda-lambdau)*pnorm*pnorm);

		//-- 4.	
		if (delta<=0) {
			//-- s=s+(lambda-2*delta/pnorm2)*p
			Alg->Vadd(weightsCntTotal, scgd->s, 1, scgd->p, (lambda-2*delta/(pnorm*pnorm)), scgd->s); Alg->Vnorm(weightsCntTotal, scgd->s, &snorm);
			//-- lambdau=2*(lambda-delta/pnorm2)
			lambdau=2*(lambda-delta/(pnorm*pnorm));
			//-- delta=-delta+lambda*pnorm2
			delta=-delta+lambda*pnorm*pnorm;
			//-- lambda=lambdau
			lambda=lambdau;
		}

		if (isnan(delta)||isinf(delta)) break;

		//-- 5.	Calclulate step size

		//-- mu=VdotV(p*r)
		Alg->VdotV(weightsCntTotal, scgd->p, scgd->r, &mu);
		//-- alpha=mu/delta
		alpha=mu/delta;

		//-- 6. Calculate comparison parameter
		EcalcG(procArgs->ds, W, &Gtse_old);
		//-- newW=W+alpha*p
		Alg->Vadd(weightsCntTotal, scgd->p, alpha, W, 1, scgd->newW); Alg->Vnorm(weightsCntTotal, scgd->newW, &newWnorm);
		//-- Gtse_new=EcalcG(newW)
		EcalcG(procArgs->ds, scgd->newW, &Gtse_new);
		//-- comp=2*delta/mu^2*(Gtse_old-Gtse_new)
		comp=2*delta/(mu*mu)*(Gtse_old-Gtse_new);

		//--7.	
		if (comp>=0) {
			//-- w=w+alpha*p
			Alg->Vadd(weightsCntTotal, W, 1, scgd->p, alpha, W);
			Alg->Vnorm(weightsCntTotal, W, &Wnorm);
			//-- prevR=r
			Alg->Vcopy(weightsCntTotal, scgd->r, scgd->prev_r);
			//-- r=-dEcalcG(w)
			dEcalcG(procArgs->ds, W, scgd->r); Alg->Vscale(weightsCntTotal, scgd->r, -1, scgd->r);
			lambdau=0;
			success=true;

			//-- 7a
			if ((k+1)%weightsCntTotal==0) {
				//-- p=r
				Alg->Vcopy(weightsCntTotal, scgd->r, scgd->p);
			} else {
				//-- calc rnorm
				Alg->Vnorm(weightsCntTotal, scgd->r, &rnorm);
				//-- calc VdotV(r, prev_r)
				Alg->VdotV(weightsCntTotal, scgd->r, scgd->prev_r, &rdotprevr);
				//-- beta=(rnorm^2-r*prevR)/mu
				beta=((rnorm*rnorm)-rdotprevr)/mu;
				//-- p=r+beta*p
				Alg->Vadd(weightsCntTotal, scgd->r, 1, scgd->p, beta, scgd->p); Alg->Vnorm(weightsCntTotal, scgd->p, &pnorm);
			}

			//-- 7b
			if (comp>=0.75) lambda/=2;

		} else {
			lambdau=lambda;
			success=false;
		}

		//-- 8.
		if (comp<0.25) lambda*=4;

		//-- 9. 

		//-- recalc rnorm
		Alg->Vnorm(weightsCntTotal, scgd->r, &rnorm);
		//-- display progress
		showEpochStatsG(k, kstart, success, Gtse_new);
		//-- save scgd->log
		if (persistor->saveInternalsFlag) {
			scgd->log->delta[k]=delta;
			scgd->log->mu[k]=mu;
			scgd->log->alpha[k]=alpha;
			scgd->log->beta[k]=beta;
			scgd->log->lambda[k]=lambda;
			scgd->log->lambdau[k]=lambdau;
			scgd->log->Gtse_old[k]=Gtse_old;
			scgd->log->Gtse_new[k]=Gtse_new;
			scgd->log->comp[k]=comp;
			scgd->log->pnorm[k]=pnorm;
			scgd->log->rnorm[k]=rnorm;
			scgd->log->dwnorm[k]=0;
			scgd->log->iterationsCnt++;
		}

		k++;
	} while ((rnorm>0)&&(k<parms->SCGDmaxK));
	
	//-- persist scgd->log
	if (persistor->saveInternalsFlag) safecall(persistor, saveCoreNNInternalsSCGD, pid, tid, k-1, scgd->log->delta, scgd->log->mu, scgd->log->alpha, scgd->log->beta, scgd->log->lambda, scgd->log->lambdau, scgd->log->Gtse_old, scgd->log->Gtse_new, scgd->log->comp, scgd->log->pnorm, scgd->log->rnorm, scgd->log->dwnorm);

	return k;
}
void sNN::loadWholeDataSet() {
	int sampleSize=procArgs->ds->samplesCnt*procArgs->ds->shape->sampleLen*procArgs->ds->shape->featuresCnt;
	safecall(Alg, myMalloc, &sample_d, sampleSize);
	safecall(Alg, h2d, sample_d, procArgs->ds->sampleBFS, (int)(sampleSize*sizeof(numtype)), false);

	int targetSize=procArgs->ds->samplesCnt*procArgs->ds->shape->predictionLen*procArgs->ds->shape->featuresCnt;
	safecall(Alg, myMalloc, &target_d, targetSize);
	safecall(Alg, h2d, target_d, procArgs->ds->targetBFS, (int)(targetSize*sizeof(numtype)), false);
}
