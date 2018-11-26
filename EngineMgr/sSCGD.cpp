#include "sSCGD.h"

sSCGD::sSCGD(int Wcnt){
	p=(numtype*)malloc(Wcnt*sizeof(numtype));
	r=(numtype*)malloc(Wcnt*sizeof(numtype));
	s=(numtype*)malloc(Wcnt*sizeof(numtype));
	dW=(numtype*)malloc(Wcnt*sizeof(numtype));
	TotdW=(numtype*)malloc(Wcnt*sizeof(numtype));
	newW=(numtype*)malloc(Wcnt*sizeof(numtype));
	oldW=(numtype*)malloc(Wcnt*sizeof(numtype));
	prev_r=(numtype*)malloc(Wcnt*sizeof(numtype));
	alphap=(numtype*)malloc(Wcnt*sizeof(numtype));
	bp=(numtype*)malloc(Wcnt*sizeof(numtype));
	lp=(numtype*)malloc(Wcnt*sizeof(numtype));
	ap=(numtype*)malloc(Wcnt*sizeof(numtype));
	dE0=(numtype*)malloc(Wcnt*sizeof(numtype));
	dE1=(numtype*)malloc(Wcnt*sizeof(numtype));
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

void sSCGD::BP(int pid, int tid, sAlgebra* Alg, int levelsCnt, int* nodesCnt, int netWcnt, numtype* netW, numtype targetMSE) {


	int k;

	numtype sigma, delta, mu, alpha, beta = 0, b1, b2;
	numtype lambda, lambdau;
	numtype pnorm, pnorm2;
	numtype e_old=0, e_new=0, comp;
	bool success;
	numtype epsilon = targetMSE/nodesCnt[levelsCnt-1];

	//BackupW(NN, Mx->NN.W, t4);
	Alg->Vinit(netWcnt, TotdW, 0, 0);

	dEcalc(NN, Mx, true);

	//-- 1. Choose initial vector w ; p=r=-E'(w)
	Alg->Vcopy(netWcnt, LVV_GdJdW, p); Alg->Vscale(netWcnt, p, -1, p);
	Alg->Vcopy(netWcnt, p, r);

	success = true;
	sigma = 1e-4;
	lambda = 1e-6; lambdau = 0;

	k = 0;
	do {
		Alg->Vnorm(netWcnt, r, &rnorm);
		Alg->Vnorm(netWcnt, p, &pnorm);
		pnorm2 = pow(pnorm, 2);

		//-- 2. if success=true Calculate second-order  information (s and delta)
		if (success) {

			//-- non-Hessian approximation
			sigma = sigma/pnorm;
			//-- get dE0 (dJdW at current W)
			Alg->Vcopy(netWcnt, LVV_GdJdW, dE0);
			//-- get dE1 (dJdW at W+sigma*p)
			Alg->Vscale(netWcnt, p, sigma, sigmap);
			Alg->Vadd(netWcnt, LVV_W, 1, sigmap, 1, newW);
			//=================================
			dEcalc(NN, Mx, true, true, false, newW, dE1);
			//-- calc s
			Alg->Vadd(netWcnt, dE1, 1, dE0, -1, dE);
			Alg->Vscale(netWcnt, dE, 1/sigma, s);
			//-- calc delta
			delta = Alg->VdotV(netWcnt, p, s);
		}

		//-- 3. scale s and delta

		//--- 3.1 s=s+(lambda-lambdau)*p
		Alg->Vadd(netWcnt,s, 1, p, (lambda-lambdau), s);
		//--- 3.2 delta=delta+(lambda-lambdau)*|p|^2
		delta += (lambda-lambdau)*pnorm2;

		//-- 4. if delta<=0 (i.e. Hessian is not positive definite) , then make it positive
		if (delta<=0) {
			//-- adjust s
			Alg->Vadd(netWcnt, s, 1, p, (lambda-2*delta/pnorm2), s);
			//-- adjust lambdau
			lambdau = 2*(lambda-delta/pnorm2);
			//-- adjust delta
			delta = -delta+lambda*pnorm2;
			//-- adjust lambda
			lambda = lambdau;
		}

		//-- 5. Calculate step size
		Alg->VdotV(netWcnt, p, r, &mu);
		alpha = mu/delta;

		//-- 6. Comparison parameter

		//--- 6.1 calc newW=w+alpha*p , which will also be used in (7)
		Alg->Vscale(netWcnt, p, alpha, dW);
		Alg->Vadd(netWcnt, LVV_W, 1, dW, 1, newW);

		if (success) Alg->Vcopy(netWcnt, LVV_W, oldW);
		//--- 6.2 E(w)
		e_old = Ecalc(NN, Mx, -1, oldW);
		//--- 6.3 E(w+dw) is calculated by E_at_w()
		e_new = Ecalc(NN, Mx, -1, newW);

		//--- 6.4 comp=2*delta*(e_old-e_new)/mu^2
		comp = 2*delta*(e_old-e_new)/pow(mu, 2);

		if (comp>=0) {
			//-- 7. Update weight vector

			//-- dW = alpha * p
			Alg->Vscale(netWcnt, p, alpha, dW);
			//-- W = W + dW
			Alg->Vadd(netWcnt, LVV_W, 1, dW, 1, LVV_W);
			//-- TotdW = TotdW + dW
			Alg->Vadd(netWcnt, TotdW, 1, dW, 1, TotdW);
			//-- 7.1 recalc  dJdW
			dEcalc(NN, Mx, true, true);

			//-- save r, and calc new r
			Alg->Vcopy(netWcnt, r, prev_r);
			Alg->Vcopy(netWcnt, LVV_GdJdW, r); Alg->Vscale(netWcnt, r, -1, r);

			//-- reset lambdau
			lambdau = 0; success = true;

			//-- 7a. if k mod N = 0 then restart algorithm, else create new conjugate direction
			if (((k+1)%NN->NodesCountTotal)==0) {
				Alg->Vcopy(netWcnt, r, p);
			} else {
				numtype _b;
				Alg->Vnorm(netWcnt, r, &_b);
				b1 = pow(_b, 2);
				Alg->VdotV(netWcnt, r, prev_r, &b2);
				beta = (b1-b2)/mu;
				//-- p = r + beta*p
				Alg->Vadd(netWcnt, p, beta, r, 1, p);
			}
			//-- 7b. if comp>=0.75 reduce scale parameter
			if (comp>=0.75) lambda = lambda/2;

		} else {
			//-- a reduction in error is not possible.
			lambdau = lambda;
			success = false;
		}

		//-- 8. if comp<0.25 then increase scale parameter
		if (comp<0.25) lambda = lambda*4;

		//-- 9. if the steepest descent direction r>epsilon and success=true, then set k=k+1 and go to 2, else terminate and return w as the desired minimum
		Alg->Vnorm(netWcnt, r, &rnorm);
		//-- display progress
		WaitForSingleObject(Mx->ScreenMutex, 10);
		gotoxy(0, Mx->ScreenPos); printf("\rProcess %6d, Thread %6d, Iteration %6d , success=%s, rnorm=%f , norm_ge=%f", pid, tid, k, (success) ? "TRUE " : "FALSE", rnorm, Mx->NN.norm_ge);
		ReleaseMutex(Mx->ScreenMutex);

		//if (DebugParms->SaveInternals>0) SaveCoreData_SCGD(NNLogs, pid, tid, 0, Mx->sampleid, Mx->BPCount, k, Mx->SCGD_progK, delta, mu, alpha, beta, lambda, lambdau, pnorm, rnorm, Mx->NN.norm_ge, Vnorm(netWcnt, dW), comp);

		k++; Mx->SCGD_progK++;
	} while ((rnorm>0)&&(k<NN->SCGDmaxK));

	//RestoreW(NN, Mx->NN.W, t4);

}