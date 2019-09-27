
		Alg->Vinit(1, tse, 0, 0); for (b=0; b<procArgs->batchCnt; b++) safecallSilent(this, ForwardPass, trainArgs->ds, b, false); Alg->d2h(&tse_h, tse, 1*sizeof(numtype), false);