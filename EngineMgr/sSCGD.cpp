#include "sSCGD.h"

sSCGD::sSCGD(int Wcnt){
	p=(numtype*)malloc(Wcnt*sizeof(numtype));
	r=(numtype*)malloc(Wcnt*sizeof(numtype));
	s=(numtype*)malloc(Wcnt*sizeof(numtype));
	dW=(numtype*)malloc(Wcnt*sizeof(numtype));
	TotdW=(numtype*)malloc(Wcnt*sizeof(numtype));
	newW=(numtype*)malloc(Wcnt*sizeof(numtype));
	oldW=(numtype*)malloc(Wcnt*sizeof(numtype));
	GdJdW=(numtype*)malloc(Wcnt*sizeof(numtype));
	prev_r=(numtype*)malloc(Wcnt*sizeof(numtype));
	alphap=(numtype*)malloc(Wcnt*sizeof(numtype));
	bp=(numtype*)malloc(Wcnt*sizeof(numtype));
	lp=(numtype*)malloc(Wcnt*sizeof(numtype));
	ap=(numtype*)malloc(Wcnt*sizeof(numtype));
	dE0=(numtype*)malloc(Wcnt*sizeof(numtype));
	dE1=(numtype*)malloc(Wcnt*sizeof(numtype));
	dE=(numtype*)malloc(Wcnt*sizeof(numtype));
	E0=(numtype*)malloc(Wcnt*sizeof(numtype));
	E1=(numtype*)malloc(Wcnt*sizeof(numtype));
	E=(numtype*)malloc(Wcnt*sizeof(numtype));
	sigmap=(numtype*)malloc(Wcnt*sizeof(numtype));
}
sSCGD::~sSCGD() {
	free(p);
	free(r);
	free(s);
	free(dW);
	free(TotdW);
	free(newW);
	free(oldW);
	free(GdJdW);
	free(prev_r);
	free(alphap);
	free(bp);
	free(lp);
	free(ap);
	free(dE0);
	free(dE1);
	free(dE);
	free(E0);
	free(E1);
	free(E);
	free(sigmap);
}
