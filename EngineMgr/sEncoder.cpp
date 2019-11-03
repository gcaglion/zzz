#include "sEncoder.h"

sEncoder::sEncoder(sObjParmsDef, int inputSize_, int levelsCnt_, float* levelRatio_, int* activationFunction_, numtype learningRate_, numtype learningMomentum_) : sObj(sObjParmsVal) {
	int l;

	activationFunction=activationFunction_; learningRate=learningRate_; learningMomentum=learningMomentum_;

	nodesCnt=(int*)malloc(2*levelsCnt_*sizeof(int));
	weightsCnt=(int*)malloc((2*levelsCnt_-1)*sizeof(int));
	nodesCnt[0]=inputSize_;
	for (l=1; l<levelsCnt_; l++) {
		nodesCnt[l]=(int)(nodesCnt[l-1]*levelRatio_[l-1]); nodesCntTotal+=nodesCnt[l];
		weightsCnt[l-1]=nodesCnt[l]*nodesCnt[l-1]; weightsCntTotal+=weightsCnt[l-1];
	}
	for (l=0; l<(levelsCnt_-1); l++) {
		nodesCnt[levelsCnt_+l]=nodesCnt[levelsCnt_-l-2];
		weightsCnt[levelsCnt_+l-1]=nodesCnt[levelsCnt_-l-2]*nodesCnt[levelsCnt_-l-1];
	}
	levelsCnt=2*levelsCnt_-1;

	levelFirstNode=(int*)malloc(levelsCnt*sizeof(int));
	levelFirstWeight=(int*)malloc((levelsCnt-1)*sizeof(int));
	for (l=0; l<levelsCnt; l++) {
		levelFirstNode[l]=0;
		for (int ll=0; ll<l; ll++) {
			levelFirstNode[l]+=nodesCnt[ll];
		}
	}
	for (l=0; l<(levelsCnt-1); l++) {
		levelFirstWeight[l]=0;
		for (int ll=0; ll<l; ll++) {
			levelFirstWeight[l]+=weightsCnt[ll];
		}
	}

	nodesCntTotal=0; for (l=0; l<(levelsCnt); l++) nodesCntTotal+=nodesCnt[l];
	weightsCntTotal=0; for (l=0; l<(levelsCnt-1); l++) weightsCntTotal+=weightsCnt[l];


	//safespawn(Alg, newsname("Alg"), defaultdbg);
	Alg = new sAlgebra(nullptr, newsname("Alg"), defaultdbg, nullptr);

	Alg->myMalloc(&a, nodesCntTotal);
	Alg->myMalloc(&F, nodesCntTotal);
	Alg->myMalloc(&dF, nodesCntTotal);
	Alg->myMalloc(&edF, nodesCntTotal);
	Alg->myMalloc(&er, nodesCnt[levelsCnt-1]);
	Alg->myMalloc(&W, weightsCntTotal);
	Alg->myMalloc(&dW, weightsCntTotal);
	Alg->myMalloc(&dJdW, weightsCntTotal);

	Alg->myMalloc(&sse, 1);
	Alg->myMalloc(&bse, 1);
	Alg->myMalloc(&ese, 1);

}
sEncoder::~sEncoder() {
	free(nodesCnt);
	free(weightsCnt);
	free(levelFirstNode);
	free(levelFirstWeight);

	Alg->myFree(a); Alg->myFree(F); Alg->myFree(dF); Alg->myFree(edF); Alg->myFree(er);
	Alg->myFree(W); Alg->myFree(dW); Alg->myFree(dJdW);
}

void sEncoder::FF() {
	for (int l=0; l<(levelsCnt-1); l++) {
		int Ay=nodesCnt[l+1];
		int Ax=nodesCnt[l];
		numtype* A=&W[levelFirstWeight[l]];
		int By=nodesCnt[l];
		int Bx=1;
		numtype* B=&F[levelFirstNode[l]];
		numtype* C=&a[levelFirstNode[l+1]];

		//-- actual feed forward ( W10[nc1 X nc0] X F0[nc0 X batchSize] => a1 [nc1 X batchSize] )
		Alg->MbyM(Ay, Ax, 1, false, A, By, Bx, 1, false, B, C);
		//-- activation sets F[l+1] and dF[l+1]
		Activate(l+1);
	}
}
void sEncoder::Activate(int level) {
	// sets F, dF
	int retf, retd;
	int nc=nodesCnt[level];
	numtype* va=&a[levelFirstNode[level]];
	numtype* vF=&F[levelFirstNode[level]];
	numtype* vdF=&dF[levelFirstNode[level]];

	switch (activationFunction[level]) {
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
void sEncoder::dEcalc() {
	int Ay, Ax, Astart, By, Bx, Bstart, Cy, Cx, Cstart;
	numtype* A; numtype* B; numtype* C;

	for (int l = (levelsCnt-1); l>0; l--) {
		if (l==(levelsCnt-1)) {
			//-- top level only
			Alg->VbyV2V(nodesCnt[l], er, &dF[levelFirstNode[l]], &edF[levelFirstNode[l]]);	// edF(l) = e * dF(l)
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
void sEncoder::showEpochStats(int e, int maxe, DWORD eStart_, numtype epochMSE) {
	//=======  !!!! CHECK FOR PERFORMANCE DEGRADATION !!!  ========
	char remainingTimeS[TIMER_ELAPSED_FORMAT_LEN];
	int duration;

	duration=timeGetTime()-eStart_;
	DWORD remainingms=(maxe-e)*duration;
	SgetElapsed(remainingms, remainingTimeS);
	//=======  !!!! CHECK FOR PERFORMANCE DEGRADATION !!!  ========
	sprintf_s(dbg->msg, DBG_MSG_MAXLEN, "\rTestId %3d, Process %6d, Thread %6d, Epoch %6d/%6d , Training MSE=%1.10f , Validation MSE=%1.10f, duration=%d ms , remaining: %s", 0, pid, tid, e, maxe, epochMSE, 0.0f, duration, remainingTimeS);

	//if (dbg->dbgtoscreen) {
	if (GUIreporter!=nullptr) {
		(*GUIreporter)(20, dbg->msg);
	} else {
		gotoxy(0, 0);
		printf(dbg->msg);
	}
	//}

}

void sEncoder::train(int samplesCnt, int sampleLen, int featuresCnt, numtype* sample, int batchSize, int maxEpochs) {
	pid=GetCurrentProcessId(); tid=GetCurrentThreadId();
	DWORD epoch_starttime;

	numtype ese_h;
	numtype* mseT=(numtype*)malloc(maxEpochs*sizeof(numtype));

	int batchCnt=(int)floor(samplesCnt/batchSize);
	for (int e=0; e<maxEpochs; e++) {
		epoch_starttime=timeGetTime();
		Alg->Vinit(1, ese, 0, 0);
		for (int b=0; b<batchCnt; b++) {
			Alg->Vinit(1, bse, 0, 0);
			for (int s=0; s<batchSize; s++) {
				Alg->Vinit(1, sse, 0, 0);

				//-- load one sample
				Alg->d2d(&F[0], &sample[b*batchSize*sampleLen*featuresCnt+s*sampleLen*featuresCnt], sampleLen*featuresCnt*sizeof(numtype));
				Alg->d2d(&F[levelFirstNode[levelsCnt-1]], &sample[b*batchSize*sampleLen*featuresCnt+s*sampleLen*featuresCnt], sampleLen*featuresCnt*sizeof(numtype));

				//-- ff
				FF();

				//-- calc error (output layer vs. input layer)
				Alg->Vadd(nodesCnt[levelsCnt-1], &F[levelFirstNode[levelsCnt-1]], 1, &F[0], -1, er);
				Alg->Vssum(nodesCnt[levelsCnt-1], er, sse);
				Alg->Vadd(1, bse, 1, sse, 1, bse);				//-- batch se += sample se
				Alg->Vadd(1, ese, 1, sse, 1, ese);				//-- epoch se += sample se

																//-- bp
				dEcalc();
				Alg->Vadd(weightsCntTotal, dW, learningMomentum, dJdW, -learningRate, dW);	//-- dW = LM*dW - LR*dJdW
			}
			//-- weights update at the end of the batch
			Alg->Vadd(weightsCntTotal, W, 1, dW, 1, W);
		}
		//-- 1.2. calc epoch MSE (for ALL batches), and check criteria for terminating training (targetMSE, Divergence)
		Alg->d2h(&ese_h, ese, 1*sizeof(numtype), false);
		mseT[e]=ese_h/nodesCnt[levelsCnt-1]/batchCnt;
		showEpochStats(e, maxEpochs, epoch_starttime, mseT[e]);
	}

}

