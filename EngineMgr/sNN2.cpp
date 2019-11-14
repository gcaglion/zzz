#include "sNN2.h"
//#include <vld.h>

void sNN2::setLayout() {
	int l, nl;


	//-- 0.3. set nodesCnt (single sample)
	nodesCnt[0] = layout->inputCnt;
	nodesCnt[outputLevel] = layout->outputCnt;
	for (nl = 0; nl<(parms->levelsCnt-2); nl++) nodesCnt[nl+1] = (int)floor(nodesCnt[nl]*parms->levelRatio[nl]);

	//-- add context neurons
	if (parms->useContext) {
		for (nl = outputLevel; nl>0; nl--) nodesCnt[nl-1] += nodesCnt[nl];
	}

	//-- add bias neurons
	if (parms->useBias) {
		for (l=0; l<outputLevel; l++) nodesCnt[l]++;
	}

	//-- 0.2. calc nodesCntTotal
	nodesCntTotal=0;
	for (l=0; l<parms->levelsCnt; l++) nodesCntTotal+=nodesCnt[l];

	//-- 0.3. weights count
	weightsCntTotal=0;
	for (l=0; l<(outputLevel); l++) {
		weightsCnt[l]=nodesCnt[l]*nodesCnt[l+1];
		weightsCntTotal+=weightsCnt[l];
	}

	//-- 0.4. set first node and first weight for each layer
	for (l=0; l<parms->levelsCnt; l++) {
		levelFirstNode[l]=0;
		for (int ll=0; ll<l; ll++) {
			levelFirstNode[l]+=nodesCnt[ll];
		}
	}
	for (l=0; l<outputLevel; l++) {
		levelFirstWeight[l]=0;
		for (int ll=0; ll<l; ll++) {
			levelFirstWeight[l]+=weightsCnt[ll];
		}
	}

	//-- ctxStart[] can only be defined after levelFirstNode has been defined.
	if (parms->useContext) {
		for (nl=0; nl<(outputLevel); nl++) ctxStart[nl]=levelFirstNode[nl+1]-nodesCnt[nl+1];
	}

}

void sNN2::sNNcommon(sNNparms* NNparms_) {
	parms=NNparms_;
	outputLevel=parms->levelsCnt-1;
	//-- allocate level-specific parameters
	nodesCnt=(int*)malloc(parms->levelsCnt*sizeof(int));
	levelFirstNode=(int*)malloc(parms->levelsCnt*sizeof(int));
	ctxStart=(int*)malloc(parms->levelsCnt*sizeof(int));
	weightsCnt=(int*)malloc((outputLevel)*sizeof(int));
	levelFirstWeight=(int*)malloc((outputLevel)*sizeof(int));

	setLayout();
	mallocNeurons();
	initNeurons();
	createWeights();

}

sNN2::sNN2(sObjParmsDef, sCoreLayout* layout_, sCoreLogger* persistor_, sNNparms* NNparms_) : sCore(sObjParmsVal, nullptr, "", layout_, persistor_) {
	sNNcommon(NNparms_);
}
sNN2::sNN2(sCfgObjParmsDef, sCoreLayout* layout_, sNNparms* NNparms_) : sCore(sCfgObjParmsVal, layout_) {
	sNNcommon(NNparms_);
}
sNN2::~sNN2() {
	Alg->myFree(a);
	Alg->myFree(F);
	Alg->myFree(dF);
	Alg->myFree(edF);
	Alg->myFree(e);
	Alg->myFree(u);
	Alg->myFree(tse);
	Alg->myFree(se);
	Alg->myFree(W);
	Alg->myFree(prevW);
	Alg->myFree(dW);
	Alg->myFree(dJdW);

	free(nodesCnt);
	free(weightsCnt);
	free(ctxStart);
	free(levelFirstNode);
	free(levelFirstWeight);
}

void sNN2::mallocNeurons() {
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
void sNN2::initNeurons() {
	//--
	Alg->Vinit(nodesCntTotal, F, 0, 0);
	//---- the following are needed by cublas version of MbyM
	Alg->Vinit(nodesCntTotal, a, 0, 0);
	Alg->Vinit(nodesCntTotal, dF, 0, 0);
	Alg->Vinit(nodesCntTotal, edF, 0, 0);

}
void sNN2::createWeights() {
	//-- malloc weights (on either CPU or GPU)
	Alg->myMalloc(&W, weightsCntTotal);
	Alg->myMalloc(&prevW, weightsCntTotal);
	Alg->myMalloc(&dW, weightsCntTotal);
	Alg->myMalloc(&dJdW, weightsCntTotal);
}
void sNN2::resetBias() {
	//-- bias neurons are the last neuron in each level
	int i;
	for (int l=0; l<outputLevel; l++) {
		i=levelFirstNode[l+1]-1;
		Alg->Vinit(1, &F[i], 1, 0);
	}
}

void sNN2::FF() {
	for (int l=0; l<outputLevel; l++) {
		int Ay=nodesCnt[l+1];
		int Ax=nodesCnt[l];
		numtype* A=&W[levelFirstWeight[l]];
		int By=nodesCnt[l];
		int Bx=1;
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

	if (parms->useBias) resetBias();
}
void sNN2::Activate(int level) {
	// sets F, dF
	int retf, retd;
	int nc=nodesCnt[level];
	numtype* va=&a[levelFirstNode[level]];
	numtype* vF=&F[levelFirstNode[level]];
	numtype* vdF=&dF[levelFirstNode[level]];

	switch (parms->ActivationFunction[level]) {
	case NN_ACTIVATION_TANH:
		retf=Alg->Tanh(nc, va, vF);
		retd=Alg->dTanh(nc, va, vdF);
		break;
	case NN_ACTIVATION_TANH2:
		retf=Alg->Tanh2(nc, va, vF);
		retd=Alg->dTanh2(nc, va, vdF);
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
void sNN2::Ecalc() {
	//-- sets e, bte; adds squared sum(e) to tse
	Alg->Vadd(nodesCnt[outputLevel], &F[levelFirstNode[outputLevel]], 1, u, -1, e);	// e=F[2]-u
	Alg->Vssum(nodesCnt[outputLevel], e, se);										// se=ssum(e) 
	Alg->Vadd(1, tse, 1, se, 1, tse);												// tse+=se;
}
void sNN2::dEcalc() {
	int Ay, Ax, Astart, By, Bx, Bstart, Cy, Cx, Cstart;
	numtype* A; numtype* B; numtype* C;

	for (int l = outputLevel; l>0; l--) {
		if (l==(outputLevel)) {
			//-- top level only
			Alg->VbyV2V(nodesCnt[l], e, &dF[levelFirstNode[l]], &edF[levelFirstNode[l]]);	// edF(l) = e * dF(l)
		} else {
			//-- lower levels
			Ay=nodesCnt[l+1];
			Ax=nodesCnt[l];
			Astart=levelFirstWeight[l];
			A=&W[Astart];
			By=nodesCnt[l+1];
			Bx=1;
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
		Ay=nodesCnt[l];
		Ax=1;
		Astart=levelFirstNode[l];
		A=&edF[Astart];
		By=nodesCnt[l-1];
		Bx=1;
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
void sNN2::showEpochStats(int e, DWORD eStart_) {
	//=======  !!!! CHECK FOR PERFORMANCE DEGRADATION !!!  ========
	char remainingTimeS[TIMER_ELAPSED_FORMAT_LEN];

	procArgs->duration[e]=timeGetTime()-eStart_;
	DWORD remainingms=(parms->MaxEpochs-e)*procArgs->duration[e];
	SgetElapsed(remainingms, remainingTimeS);
	//=======  !!!! CHECK FOR PERFORMANCE DEGRADATION !!!  ========
	sprintf_s(dbg->msg, DBG_MSG_MAXLEN, "\rTestId %3d, Process %6d, Thread %6d, Epoch %6d/%6d , Training MSE=%1.10f , Validation MSE=%1.10f, duration=%d ms , remaining: %s", procArgs->testid, procArgs->pid, procArgs->tid, e, parms->MaxEpochs, procArgs->mseT[e], procArgs->mseV[e], procArgs->duration[e], remainingTimeS);

	//if (dbg->dbgtoscreen) {
	if (GUIreporter!=nullptr) {
		(*GUIreporter)(20, dbg->msg);
	} else {
		gotoxy(0, procArgs->screenLine);
		printf(dbg->msg);
	}
	//}

}

volatile bool trainingBreakRequested2;
BOOL WINAPI breakTraining2(DWORD signal) {
	trainingBreakRequested2=true;
	return true;
}
void sNN2::train() {
	int l;
	DWORD epoch_starttime;
	DWORD training_starttime=timeGetTime();
	bool hasInverted=false, hasDiverged=false, hasMinimized=false;

	//-- malloc mse[maxepochs], always host-side. We need to free them, first (see issue when running without training...)
	procArgs->duration=(int*)malloc(parms->MaxEpochs*sizeof(int));
	procArgs->mseT=(numtype*)malloc(parms->MaxEpochs*sizeof(numtype));
	procArgs->mseV=(numtype*)malloc(parms->MaxEpochs*sizeof(numtype));

	//---- 0.2. Init W
	for (l=0; l<(outputLevel); l++) Alg->VinitRnd(weightsCnt[l], &W[levelFirstWeight[l]], -1/sqrtf((numtype)nodesCnt[l]), 1/sqrtf((numtype)nodesCnt[l]), Alg->cuRandH);

	//---- 0.3. Init dW, dJdW
	Alg->Vinit(weightsCntTotal, dW, 0, 0);
	Alg->Vinit(weightsCntTotal, dJdW, 0, 0);

	//-- pre-load the whole dataset (samples+targets) on GPU !
	safecall(this, loadWholeDataSet);

	if (!SetConsoleCtrlHandler(breakTraining2, TRUE)) {
		printf("\nERROR: Could not set control handler");
		return;
	}
	trainingBreakRequested2=false;

	for (epoch=0; epoch<parms->MaxEpochs; epoch++) {

		if (trainingBreakRequested2) {
			printf("\nQuit after break?");
			int c=getchar();
			procArgs->quitAfterBreak=(c=='y'||c=='Y');
			break;
		}

		//-- timing
		epoch_starttime=timeGetTime();

		Alg->Vinit(1, tse, 0, 0);
		Alg->d2d(prevW, W, weightsCntTotal*sizeof(numtype));	//-- save prev W

		for (int b=0; b<procArgs->batchCnt; b++) {
			Alg->Vinit(weightsCntTotal, dW, 0, 0);
			for (int s=0; s<procArgs->batchSize; s++) {
				//-- 1. load sample/target
				int sid=b*procArgs->batchSize*procArgs->inputCnt+s*procArgs->inputCnt;
				int tid=b*procArgs->batchSize*procArgs->outputCnt+s*procArgs->outputCnt;
				Alg->d2d(&F[0], &sample_d[sid], procArgs->inputCnt*sizeof(numtype));
				Alg->d2d(&u[0], &target_d[tid], procArgs->outputCnt*sizeof(numtype));
				//-- 2. fwd
				FF();
				//-- 3. calc e,tse
				Ecalc();
				//-- 4. calc dJdW
				dEcalc();
				//-- 5. calc dW = LM*dW - LR*dJdW
				Alg->Vadd(weightsCntTotal, dW, parms->LearningMomentum, dJdW, -parms->LearningRate, dW);
			}
			//-- 2. update W = W + dW for current batch
			Alg->Vadd(weightsCntTotal, W, 1, dW, 1, W);
		}

		//-- 1.2. calc epoch MSE (for ALL batches), and check criteria for terminating training (targetMSE, Divergence)
		Alg->d2h(&tse_h, tse, 1*sizeof(numtype), false);
		procArgs->mseT[epoch]=tse_h/procArgs->samplesCnt/nodesCnt[outputLevel];
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
	float elapsed_avg=elapsed_tot/procArgs->mseCnt;
}
void sNN2::infer(){
	DWORD inferStartTime=timeGetTime();

	//-- pre-load the whole dataset (samples+targets) on GPU !
	safecall(this, loadWholeDataSet);

	//-- 1. infer all batches with one Forward pass ( loadSamples(b)+FF()+calcErr() ). No backward pass, obviously.

	//-- timing
	inferStartTime=timeGetTime();

	Alg->Vinit(1, tse, 0, 0); 
	for (int b=0; b<procArgs->batchCnt; b++) {
		for (int s=0; s<procArgs->batchSize; s++) {
			//-- 1. load sample/target
			int sid=b*procArgs->batchSize*procArgs->inputCnt+s*procArgs->inputCnt;
			int tid=b*procArgs->batchSize*procArgs->outputCnt+s*procArgs->outputCnt;
			safecallSilent(Alg, d2d, &F[0], &sample_d[sid], procArgs->inputCnt*sizeof(numtype));
			safecallSilent(Alg, d2d, &u[0], &target_d[tid], procArgs->outputCnt*sizeof(numtype));
			//-- 2. fwd
			FF();
			//-- 3. calc e,tse
			Ecalc();
			//-- 4. Save results for current batch in batchPrediction
			Alg->d2h(&procArgs->prediction[b*nodesCnt[outputLevel]], &F[levelFirstNode[outputLevel]], nodesCnt[outputLevel]*sizeof(numtype));
		}
	}
	Alg->d2h(&tse_h, tse, 1*sizeof(numtype), false);
	procArgs->mseR=tse_h/procArgs->samplesCnt/nodesCnt[outputLevel];
}

void sNN2::loadWholeDataSet() {
	int sampleSize=procArgs->samplesCnt*procArgs->inputCnt;
	safecall(Alg, myMalloc, &sample_d, sampleSize);
	safecall(Alg, h2d, sample_d, procArgs->sample, (int)(sampleSize*sizeof(numtype)), true);

	int targetSize=procArgs->samplesCnt*procArgs->outputCnt;
	safecall(Alg, myMalloc, &target_d, targetSize);
	safecall(Alg, h2d, target_d, procArgs->target, (int)(targetSize*sizeof(numtype)), false);
}
void sNN2::saveImage(int pid, int tid, int epoch) {

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
void sNN2::loadImage(int pid, int tid, int epoch) {

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
