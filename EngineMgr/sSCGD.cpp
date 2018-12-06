#include "sSCGD.h"

sSCGDlog::sSCGDlog(int maxK_) {
	iterationsCnt=0;
	delta=(numtype*)malloc(maxK_*sizeof(numtype));
	mu=(numtype*)malloc(maxK_*sizeof(numtype));
	alpha=(numtype*)malloc(maxK_*sizeof(numtype));
	beta=(numtype*)malloc(maxK_*sizeof(numtype));
	lambda=(numtype*)malloc(maxK_*sizeof(numtype));
	lambdau=(numtype*)malloc(maxK_*sizeof(numtype));
	Gtse_old=(numtype*)malloc(maxK_*sizeof(numtype));
	Gtse_new=(numtype*)malloc(maxK_*sizeof(numtype));
	comp=(numtype*)malloc(maxK_*sizeof(numtype));
	pnorm=(numtype*)malloc(maxK_*sizeof(numtype));
	rnorm=(numtype*)malloc(maxK_*sizeof(numtype));
	dwnorm=(numtype*)malloc(maxK_*sizeof(numtype));
}
sSCGDlog::~sSCGDlog() {
	free(delta);
	free(mu);
	free(alpha);
	free(beta);
	free(lambda);
	free(lambdau);
	free(Gtse_old);
	free(Gtse_new);
	free(comp);
	free(pnorm);
	free(rnorm);
	free(dwnorm);
}

sSCGD::sSCGD(sObjParmsDef, sAlgebra* Alg_, int Wcnt, int outNcnt, int maxK) : sObj(sObjParmsVal) {
	Alg=Alg_;
	Alg->myMalloc(&p, Wcnt);
	Alg->myMalloc(&r, Wcnt);
	Alg->myMalloc(&s, Wcnt);
	Alg->myMalloc(&dW, Wcnt);
	Alg->myMalloc(&newW, Wcnt);
	Alg->myMalloc(&oldW, Wcnt);
	Alg->myMalloc(&GdJdW, Wcnt);
	Alg->myMalloc(&prev_r, Wcnt);
	Alg->myMalloc(&alphap, Wcnt);
	Alg->myMalloc(&bp, Wcnt);
	Alg->myMalloc(&lp, Wcnt);
	Alg->myMalloc(&ap, Wcnt);
	Alg->myMalloc(&dE, Wcnt);
	Alg->myMalloc(&dE0, Wcnt);
	Alg->myMalloc(&dE1, Wcnt);
	Alg->myMalloc(&E0, Wcnt);
	Alg->myMalloc(&E1, Wcnt);
	Alg->myMalloc(&E, Wcnt);
	Alg->myMalloc(&sigmap, Wcnt);
	//--
	log = new sSCGDlog(maxK);
}
sSCGD::~sSCGD() {
	Alg->myFree(p);
	Alg->myFree(r);
	Alg->myFree(s);
	Alg->myFree(dW);
	Alg->myFree(newW);
	Alg->myFree(oldW);
	Alg->myFree(GdJdW);
	Alg->myFree(prev_r);
	Alg->myFree(alphap);
	Alg->myFree(bp);
	Alg->myFree(lp);
	Alg->myFree(ap);
	Alg->myFree(dE);
	Alg->myFree(dE0);
	Alg->myFree(dE1);
	Alg->myFree(E0);
	Alg->myFree(E1);
	Alg->myFree(E);
	Alg->myFree(sigmap);
}

