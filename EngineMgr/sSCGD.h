#pragma once
#include "../common.h"
#include "../Algebra/Algebra.h"

struct sSCGD {
	numtype* p;
	numtype* r;
	numtype* s;
	numtype* dW;
	numtype* TotdW;
	numtype* newW;
	numtype* oldW;
	numtype* GdJdW;
	numtype* prev_r;
	numtype* alphap;
	numtype* bp;
	numtype* lp;
	numtype* ap;
	numtype* dE0;
	numtype* dE1;
	numtype* dE;
	numtype* E0;
	numtype* E1;
	numtype* E;
	numtype* sigmap;
	numtype pnorm;
	numtype rnorm;
	numtype dWnorm;

	sSCGD(int Wcnt);
	~sSCGD();

	//void BP(int pid, int tid, sAlgebra* Alg, int levelsCnt_, int* nodesCnt_, int netWcnt, numtype* netW, numtype targetMSE_);
};
