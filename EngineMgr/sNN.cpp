#include "sNN.h"

sNN::sNN(sCfgObjParmsDef, sCoreLayout* layout_, sNNparms* NNparms_) : sCore(sCfgObjParmsVal, layout_) {
	
	pid=GetCurrentProcessId();
	tid=GetCurrentThreadId();
	parms=NNparms_;

	//-- init Algebra / CUDA/CUBLAS/CURAND stuff
	safespawn(Alg, newsname("%s_Algebra", name->base), dbg);

	parms->MaxEpochs=0;	//-- we need this so destructor does not fail when NN object is used to run-only

						//-- bias still not working(!) Better abort until it does
	if (parms->useBias) fail("Bias still not working properly. NN creation aborted.");

	//-- set Layout. We don't have batchSampleCnt, so we set it at 1. train() and run() will set it later
	setLayout(1);
	//-- weights can be set now, as they are not affected by batchSampleCnt
	safecall(this, createWeights);

	//-- x. set scaleMin / scaleMax
	scaleMin=(numtype*)malloc(parms->levelsCnt*sizeof(int));
	scaleMax=(numtype*)malloc(parms->levelsCnt*sizeof(int));

	//-- 3. malloc device-based scalar value, to be used by reduction functions (sum, ssum, ...)
	safecall(Alg, myMalloc, &se, 1);
	safecall(Alg, myMalloc, &tse, 1);

	//-- 4. we need to malloc these here (issue when running with no training...)
	mseT=(numtype*)malloc(1*sizeof(numtype));
	mseV=(numtype*)malloc(1*sizeof(numtype));

}
sNN::~sNN() {
	Alg->myFree(se);
	Alg->myFree(tse);

	free(mseT); free(mseV);
	free(scaleMin); free(scaleMax);

	free(nodesCnt);
	free(levelFirstNode);
	free(ctxStart);

	//	free(weightsCnt);
	//	free(levelFirstWeight);
	//	free(ActivationFunction);

}

void sNN::setLayout(int batchSamplesCnt_) {
	int l, nl;
	int levelsCnt=parms->levelsCnt;
	outputLevel=levelsCnt-1;

	//-- allocate level-specific parameters
	nodesCnt=(int*)malloc(levelsCnt*sizeof(int));
	levelFirstNode=(int*)malloc(levelsCnt*sizeof(int));
	ctxStart=(int*)malloc(levelsCnt*sizeof(int));
	weightsCnt=(int*)malloc((outputLevel)*sizeof(int));
	levelFirstWeight=(int*)malloc((outputLevel)*sizeof(int));

	//-- 0.2. Input-layout->outputCnt moved here, so can be reset when called by run()
	parms->batchSamplesCnt=batchSamplesCnt_;

	//-- 0.3. set nodesCnt (single sample)
	nodesCnt[0] = layout->shape->sampleLen*layout->shape->featuresCnt*parms->batchSamplesCnt;
	nodesCnt[outputLevel] = layout->shape->predictionLen*layout->shape->featuresCnt*parms->batchSamplesCnt;
	for (nl = 0; nl<(levelsCnt-2); nl++) nodesCnt[nl+1] = (int)floor(nodesCnt[nl]*parms->levelRatio[nl]);

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
	for (l=0; l<levelsCnt; l++) nodesCntTotal+=nodesCnt[l];

	//-- 0.3. weights count
	weightsCntTotal=0;
	for (l=0; l<(outputLevel); l++) {
		weightsCnt[l]=nodesCnt[l]/parms->batchSamplesCnt*nodesCnt[l+1]/parms->batchSamplesCnt;
		weightsCntTotal+=weightsCnt[l];
	}

	//-- 0.4. set first node and first weight for each layer
	for (l=0; l<levelsCnt; l++) {
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
void sNN::setActivationFunction(int* func_) {
	for (int l=0; l<parms->levelsCnt; l++) {
		parms->ActivationFunction[l]=func_[l];
		switch (parms->ActivationFunction[l]) {
		case NN_ACTIVATION_TANH:
			scaleMin[l] = -1;
			scaleMax[l] = 1;
			break;
		case NN_ACTIVATION_EXP4:
			scaleMin[l] = 0;
			scaleMax[l] = 1;
			break;
		case NN_ACTIVATION_RELU:
			scaleMin[l] = 0;
			scaleMax[l] = 1;
			break;
		case NN_ACTIVATION_SOFTPLUS:
			scaleMin[l] = 0;
			scaleMax[l] = 1;
			break;
		default:
			scaleMin[l] = -1;
			scaleMax[l] = 1;
			break;
		}
	}
}

void sNN::FF() {
	for (int l=0; l<outputLevel; l++) {
		int Ay=nodesCnt[l+1]/parms->batchSamplesCnt;
		int Ax=nodesCnt[l]/parms->batchSamplesCnt;
		numtype* A=&W[levelFirstWeight[l]];
		int By=nodesCnt[l]/parms->batchSamplesCnt;
		int Bx=parms->batchSamplesCnt;
		numtype* B=&F[levelFirstNode[l]];
		numtype* C=&a[levelFirstNode[l+1]];

		//-- actual feed forward ( W10[nc1 X nc0] X F0[nc0 X batchSize] => a1 [nc1 X batchSize] )
		FF0start=timeGetTime(); FF0cnt++;
		//safecall(Alg->MbyM(Ay, Ax, 1, false, A, By, Bx, 1, false, B, C));
		Alg->MbyM(Ay, Ax, 1, false, A, By, Bx, 1, false, B, C);
		FF0timeTot+=((DWORD)(timeGetTime()-FF0start));

		//-- activation sets F[l+1] and dF[l+1]
		FF1start=timeGetTime(); FF1cnt++;
		//safecall(Activate(l+1));
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
	/*safecall(Vdiff(nodesCnt[outputLevel], &F[levelFirstNode[outputLevel]], 1, u, 1, e));	// e=F[2]-u
	safecall(Vssum(nodesCnt[outputLevel], e, se));										// se=ssum(e) 
	safecall(Vadd(1, tse, 1, se, 1, tse));												// tse+=se;
	*/
	Alg->Vdiff(nodesCnt[outputLevel], &F[levelFirstNode[outputLevel]], 1, u, 1, e);	// e=F[2]-u
	Alg->Vssum(nodesCnt[outputLevel], e, se);										// se=ssum(e) 
	Alg->Vadd(1, tse, 1, se, 1, tse);												// tse+=se;
}

void sNN::mallocNeurons() {
	//-- malloc neurons (on either CPU or GPU)
	safecall(Alg, myMalloc, &a, nodesCntTotal);
	safecall(Alg, myMalloc, &F, nodesCntTotal);
	safecall(Alg, myMalloc, &dF, nodesCntTotal);
	safecall(Alg, myMalloc, &edF, nodesCntTotal);
	safecall(Alg, myMalloc, &e, nodesCnt[outputLevel]);
	safecall(Alg, myMalloc, &u, nodesCnt[outputLevel]);
}
void sNN::initNeurons(){
	//--
	safecall(Alg, Vinit, nodesCntTotal, F, 0, 0);
	//---- the following are needed by cublas version of MbyM
	safecall(Alg, Vinit, nodesCntTotal, a, 0, 0);
	safecall(Alg, Vinit, nodesCntTotal, dF, 0, 0);
	safecall(Alg, Vinit , nodesCntTotal, edF, 0, 0);

	if (parms->useBias) {
		for (int l=0; l<outputLevel; l++) {
			//-- set every bias node's F=1
			safecall(Alg, Vinit, 1, &F[levelFirstNode[l]], 1, 0);
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
	//-- malloc weights (on either CPU or GPU)
	safecall(Alg, myMalloc, &W, weightsCntTotal);
	safecall(Alg, myMalloc, &prevW, weightsCntTotal);
	safecall(Alg, myMalloc, &dW, weightsCntTotal);
	safecall(Alg, myMalloc, &dJdW, weightsCntTotal);
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
			//safecall(Alg, VbyV2V, nodesCnt[l], e, &dF[levelFirstNode[l]], &edF[levelFirstNode[l]]);	// edF(l) = e * dF(l)
		} else {
			//-- lower levels
			Ay=nodesCnt[l+1]/parms->batchSamplesCnt;
			Ax=nodesCnt[l]/parms->batchSamplesCnt;
			Astart=levelFirstWeight[l];
			A=&W[Astart];
			By=nodesCnt[l+1]/parms->batchSamplesCnt;
			Bx=parms->batchSamplesCnt;
			Bstart=levelFirstNode[l+1];
			B=&edF[Bstart];
			Cy=Ax;	// because A gets transposed
			Cx=Bx;
			Cstart=levelFirstNode[l];
			C=&edF[Cstart];

			//safecall(Alg->MbyM(Ay, Ax, 1, true, A, By, Bx, 1, false, B, C));	// edF(l) = edF(l+1) * WT(l)
			//safecall(VbyV2V(nodesCnt[l], &edF[levelFirstNode[l]], &dF[levelFirstNode[l]], &edF[levelFirstNode[l]]));	// edF(l) = edF(l) * dF(l)
			Alg->MbyM(Ay, Ax, 1, true, A, By, Bx, 1, false, B, C);	// edF(l) = edF(l+1) * WT(l)
			Alg->VbyV2V(nodesCnt[l], &edF[levelFirstNode[l]], &dF[levelFirstNode[l]], &edF[levelFirstNode[l]]);	// edF(l) = edF(l) * dF(l)
		}

		//-- common	
		Ay=nodesCnt[l]/parms->batchSamplesCnt;
		Ax=parms->batchSamplesCnt;
		Astart=levelFirstNode[l];
		A=&edF[Astart];
		By=nodesCnt[l-1]/parms->batchSamplesCnt;
		Bx=parms->batchSamplesCnt;
		Bstart=levelFirstNode[l-1];
		B=&F[Bstart];
		Cy=Ay;
		Cx=By;// because B gets transposed
		Cstart=levelFirstWeight[l-1];
		C=&dJdW[Cstart];

		// dJdW(l-1) = edF(l) * F(l-1)
		Alg->MbyM(Ay, Ax, 1, false, A, By, Bx, 1, true, B, C);
		//safecall(Alg->MbyM(Ay, Ax, 1, false, A, By, Bx, 1, true, B, C));

	}

}
void sNN::WU_std(){

	//-- 1. calc dW = LM*dW - LR*dJdW
	Alg->Vdiff(weightsCntTotal, dW, parms->LearningMomentum, dJdW, parms->LearningRate, dW);
	//safecall(Vdiff(weightsCntTotal, dW, parms->LearningMomentum, dJdW, parms->LearningRate, dW));

	//-- 2. update W = W + dW for current batch
	Alg->Vadd(weightsCntTotal, W, 1, dW, 1, W);
	//safecall(Vadd(weightsCntTotal, W, 1, dW, 1, W));

}
void sNN::ForwardPass(sDataSet* ds, int batchId, bool haveTargets) {

	//-- 1. load samples (and targets, if passed) from single batch in dataset onto input layer
	LDstart=timeGetTime(); LDcnt++;
	Alg->h2d(&F[(parms->useBias) ? 1 : 0], &ds->sampleBFS[batchId*nodesCnt[0]], nodesCnt[0]*sizeof(numtype), true);
	//safecall(Alg, h2d, &F[(parms->useBias) ? 1 : 0], &ds->sampleBFS[batchId*nodesCnt[0]], nodesCnt[0]*sizeof(numtype), true);
	if (haveTargets) {
		Alg->h2d(&u[0], &ds->targetBFS[batchId*nodesCnt[outputLevel]], nodesCnt[outputLevel]*sizeof(numtype), true);
		//safecall(Alg, h2d, &u[0], &ds->targetBFS[batchId*nodesCnt[outputLevel]], nodesCnt[outputLevel]*sizeof(numtype), true);
	}
	LDtimeTot+=((DWORD)(timeGetTime()-LDstart));

	//-- 2. Feed Forward
	FFstart=timeGetTime(); FFcnt++;	
	FF();
	//safecall(FF());
	FFtimeTot+=((DWORD)(timeGetTime()-FFstart));

	//-- 3. If we have targets, Calc Error (sets e[], te, updates tse) for the whole batch
	CEstart=timeGetTime(); CEcnt++;
	if (haveTargets) {
		calcErr();
		//safecall(calcErr());
	}
	CEtimeTot+=((DWORD)(timeGetTime()-CEstart));

}
void sNN::BackwardPass(tDataSet* ds, int batchId, bool updateWeights) {

	//-- 1. BackPropagate, calc dJdW for for current batch
	BPstart=timeGetTime(); BPcnt++;
	BP_std();
	//safecall(BP_std());
	BPtimeTot+=((DWORD)(timeGetTime()-BPstart));

	//-- 2. Weights Update for current batch
	WUstart=timeGetTime(); WUcnt++;
	if (updateWeights) {
		WU_std();
		//safecall(WU_std());
	}
	WUtimeTot+=((DWORD)(timeGetTime()-WUstart));

}
bool sNN::epochMetCriteria(int epoch, DWORD starttime, bool displayProgress) {
	numtype tse_h;	// total squared error copid on host at the end of each eopch

	Alg->d2h(&tse_h, tse, sizeof(numtype));
	mseT[epoch]=tse_h/nodesCnt[outputLevel]/batchCnt_;
	mseV[epoch]=0;	// TO DO !
	if(displayProgress) printf("\rpid=%d, tid=%d, epoch %d, Training TSE=%f, MSE=%1.10f, duration=%d ms", pid, tid, epoch, tse_h, mseT[epoch], (timeGetTime()-starttime));
	if (mseT[epoch]<parms->TargetMSE) return true;
	if ((parms->StopOnDivergence && epoch>1&&mseT[epoch]>mseT[epoch-1])) return true;
	if ((epoch%parms->NetSaveFreq)==0) {
		//-- TO DO ! (callback?)
	}

	return false;
}
void sNN::train(sDataSet* trainSet) {
	int l;
	DWORD epoch_starttime;
	DWORD training_starttime=timeGetTime();
	int epoch, b;

	//-- set batch count and batchSampleCnt for the network from dataset
	parms->batchSamplesCnt=trainSet->batchSamplesCnt;
	batchCnt_=trainSet->batchCnt;
	//-- set Layout. This should not change weightsCnt[] at all, just nodesCnt[]
	setLayout(parms->batchSamplesCnt);

	//-- 0. malloc + init neurons
	safecall(this, mallocNeurons);
	safecall(this, initNeurons);

	//-- malloc mse[maxepochs], always host-side. We need to free them, first (see issue when running without training...)
	free(mseT); mseT=(numtype*)malloc(parms->MaxEpochs*sizeof(numtype));
	free(mseV); mseV=(numtype*)malloc(parms->MaxEpochs*sizeof(numtype));

	//---- 0.2. Init W
	for (l=0; l<(outputLevel); l++) Alg->VinitRnd(weightsCnt[l], &W[levelFirstWeight[l]], -1/sqrtf((numtype)nodesCnt[l]), 1/sqrtf((numtype)nodesCnt[l]), Alg->cuRandH);
	//safecall(dumpArray(weightsCntTotal, &W[0], "C:/temp/referenceW/initW.txt"));
	//safecall(loadArray(weightsCntTotal, &W[0], "C:/temp/referenceW/initW_4F.txt"));

	//---- 0.3. Init dW, dJdW
	safecall(Alg, Vinit, weightsCntTotal, dW, 0, 0);
	safecall(Alg, Vinit, weightsCntTotal, dJdW, 0, 0);

	//-- 1. for every epoch, train all batch with one Forward pass ( loadSamples(b)+FF()+calcErr() ), and one Backward pass (BP + calcdW + W update)
	for (epoch=0; epoch<parms->MaxEpochs; epoch++) {

		//-- timing
		epoch_starttime=timeGetTime();

		//-- 1.0. reset epoch tse
		Alg->Vinit(1, tse, 0, 0);
		//safecall(Vinit(1, tse, 0, 0));

		//-- 1.1. train one batch at a time
		for (b=0; b<batchCnt_; b++) {

			//-- forward pass, with targets
			ForwardPass(trainSet, b, true);
			//safecall(ForwardPass(trainSet, b, true));

			//-- backward pass, with weights update
			BackwardPass(trainSet, b, true);
			//safecall(BackwardPass(trainSet, b, true));

		}

		//-- 1.2. calc and display epoch MSE (for ALL batches), and check criteria for terminating training (targetMSE, Divergence)
		if (epochMetCriteria(epoch, epoch_starttime)) break;

	}
	ActualEpochs=epoch-((epoch>parms->MaxEpochs)?1:0);

	//-- 2. test run. need this to make sure all batches pass through the net with the latest weights, and training targets
	TRstart=timeGetTime(); TRcnt++;
	Alg->Vinit(1, tse, 0, 0);
	//safecall(Vinit(1, tse, 0, 0));
	for (b=0; b<batchCnt_; b++) ForwardPass(trainSet, b, true);
	//for (b=0; b<batchCnt; b++) safecall(ForwardPass(trainSet, b, true));
	TRtimeTot+=((DWORD)(timeGetTime()-TRstart));

	//-- calc and display final epoch MSE
	printf("\n"); epochMetCriteria(ActualEpochs-1, epoch_starttime); printf("\n");


/*	float elapsed_tot=(float)timeGetTime()-(float)training_starttime;
	float elapsed_avg=elapsed_tot/ActualEpochs;
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
void sNN::run(sDataSet* runSet) {

	//-- set Neurons Layout based on batchSampleCount of run set
	parms->batchSamplesCnt=runSet->batchSamplesCnt;
	batchCnt_=runSet->batchCnt;
	setLayout(runSet->batchSamplesCnt);

	//-- malloc + init neurons
	safecall(this, mallocNeurons);
	safecall(this, initNeurons);

	//-- reset tse=0
	safecall(Alg, Vinit, 1, tse, 0, 0);

	//-- batch run
	for (int b=0; b<batchCnt_; b++) {

		//-- 1.1.1.  load samples/targets onto GPU
		Alg->h2d(&F[(parms->useBias) ? 1 : 0], &runSet->sampleBFS[b*nodesCnt[0]], nodesCnt[0]*sizeof(numtype), true);
		//safecall(Alg->h2d(&F[(parms->useBias) ? 1 : 0], &runSet->sampleBFS[b*layout->dataShape->inputCnt], layout->dataShape->inputCnt*sizeof(numtype), true));
		Alg->h2d(&u[0], &runSet->targetBFS[b*nodesCnt[outputLevel]], nodesCnt[outputLevel]*sizeof(numtype), true);
		//safecall(Alg->h2d(&u[0], &runSet->targetBFS[b*layout->dataShape->outputCnt], layout->dataShape->outputCnt*sizeof(numtype), true));

		//-- 1.1.2. Feed Forward
		//safecall(FF());
		FF();

		//-- 1.1.3. copy last layer neurons (on dev) to prediction (on host)
		Alg->d2h(&runSet->predictionBFS[b*nodesCnt[outputLevel]], &F[levelFirstNode[outputLevel]], nodesCnt[outputLevel]*sizeof(numtype));
		//safecall(Alg->d2h(&runSet->predictionBFS[b*layout->dataShape->outputCnt], &F[levelFirstNode[outputLevel]], layout->dataShape->outputCnt*sizeof(numtype)));

		calcErr();
		//safecall(calcErr());
	}

	//-- calc and display final epoch MSE
	numtype tse_h;	// total squared error copid on host at the end of the run
	Alg->d2h(&tse_h, tse, sizeof(numtype));
	numtype mseR=tse_h/nodesCnt[outputLevel]/batchCnt_;
	printf("\npid=%d, tid=%d, Run final MSE=%1.10f\n", pid, tid, mseR);

	//-- convert prediction from BFS to SFB (fol all batches at once)
	runSet->BFS2SFBfull(runSet->shape->predictionLen, runSet->predictionBFS, runSet->predictionSFB);
	//-- extract first bar only from target/prediction SFB
	safecall(Alg, getMcol, runSet->batchCnt*runSet->batchSamplesCnt*runSet->selectedFeaturesCnt, runSet->shape->predictionLen, runSet->targetSFB, 0, runSet->target0, true);
	safecall(Alg, getMcol, runSet->batchCnt*runSet->batchSamplesCnt*runSet->selectedFeaturesCnt, runSet->shape->predictionLen, runSet->predictionSFB, 0, runSet->prediction0, true);


	//-- feee neurons()
	destroyNeurons();
}
