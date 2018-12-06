#pragma once
#include "../common.h"
#include "../Algebra/Algebra.h"
#include "../BaseObj/sObj.h"

struct sSCGDlog {
	int iterationsCnt;
	numtype* delta;
	numtype* mu;
	numtype* alpha;
	numtype* beta;
	numtype* lambda;
	numtype* lambdau;
	numtype* Gtse_old;
	numtype* Gtse_new;
	numtype* comp;
	numtype* pnorm;
	numtype* rnorm;
	numtype* dwnorm;

	sSCGDlog(int maxK_);
	~sSCGDlog();
};

struct sSCGD : sObj {
	sAlgebra* Alg;

	numtype* p;
	numtype* r;
	numtype* s;
	numtype* dW;
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

	sSCGDlog* log;

	sSCGD(sObjParmsDef, sAlgebra* Alg_, int Wcnt, int outNcnt, int maxK);
	~sSCGD();

	//void BP(int pid, int tid, sAlgebra* Alg, int levelsCnt_, int* nodesCnt_, int netWcnt, numtype* netW, numtype targetMSE_);
};
