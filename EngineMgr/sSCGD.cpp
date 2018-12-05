#include "sSCGD.h"

sSCGD::sSCGD(sObjParmsDef, sAlgebra* Alg_, int Wcnt, int outNcnt) : sObj(sObjParmsVal) {
	Alg=Alg_;
	Alg->myMalloc(&p, Wcnt);
	Alg->myMalloc(&r, Wcnt);
	Alg->myMalloc(&s, Wcnt);
	Alg->myMalloc(&dW, Wcnt);
	Alg->myMalloc(&TotdW, Wcnt);
	Alg->myMalloc(&newW, Wcnt);
	Alg->myMalloc(&oldW, Wcnt);
	Alg->myMalloc(&GdJdW, Wcnt);
	Alg->myMalloc(&prev_r, Wcnt);
	Alg->myMalloc(&alphap, Wcnt);
	Alg->myMalloc(&bp, Wcnt);
	Alg->myMalloc(&lp, Wcnt);
	Alg->myMalloc(&ap, Wcnt);
	//Alg->myMalloc(&gse, outNcnt);
	Alg->myMalloc(&dE, Wcnt);
	Alg->myMalloc(&dE0, Wcnt);
	Alg->myMalloc(&dE1, Wcnt);
	Alg->myMalloc(&E0, Wcnt);
	Alg->myMalloc(&E1, Wcnt);
	Alg->myMalloc(&E, Wcnt);
	Alg->myMalloc(&sigmap, Wcnt);
}
sSCGD::~sSCGD() {
	Alg->myFree(p);
	Alg->myFree(r);
	Alg->myFree(s);
	Alg->myFree(dW);
	Alg->myFree(TotdW);
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
	//Alg->myFree(gse);
	Alg->myFree(dE1);
	Alg->myFree(dE);
	Alg->myFree(E0);
	Alg->myFree(E1);
	Alg->myFree(E);
	Alg->myFree(sigmap);
}

