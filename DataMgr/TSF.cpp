//-- Timeseries Statistical Features
#include "TSF.h"

int cmp_dbl(const void *x, const void *y) {
	numtype xx = *(numtype*)x, yy = *(numtype*)y;
	if (xx < yy) return -1;
	if (xx > yy) return  1;
	return 0;
}

numtype TSMean(int VLen, numtype* V) {
	numtype VSum = 0;
	for (int i = 0; i < VLen; i++) VSum += V[i];
	return(VSum/VLen);
}
numtype TSMeanAbsoluteDeviation(int VLen, numtype* V) {
	numtype VAvg = TSMean(VLen, V);
	numtype VSum = 0;
	for (int i = 0; i < VLen; i++) VSum += fabs(V[i]-VAvg);
	return(VSum/VLen);
}
numtype TSVariance(int VLen, numtype* V) {
	numtype VAvg = TSMean(VLen, V);
	numtype VSum = 0;
	for (int i = 0; i < VLen; i++) VSum += pow(V[i]-VAvg, 2);
	return(VSum/(VLen-1));
}
numtype TSSkewness(int VLen, numtype* V) {
	numtype VAvg = TSMean(VLen, V);
	numtype VSigma = sqrt(TSVariance(VLen, V));
	numtype VSum = 0;
	for (int i = 0; i < VLen; i++) VSum += pow((V[i]-VAvg)/VSigma, 3);
	return(VSum/VLen);
}
numtype TSKurtosis(int VLen, numtype* V) {
	numtype VAvg = TSMean(VLen, V);
	numtype VSigma = sqrt(TSVariance(VLen, V));
	numtype VSum = 0;
	for (int i = 0; i < VLen; i++) VSum += pow((V[i]-VAvg)/VSigma, 4);
	return(VSum/VLen-3);
}
numtype TSTurningPoints(int VLen, numtype* V) {
	int tp = 0;
	for (int i = 1; i < VLen; i++) {
		if ((V[i+1]-V[i])*(V[i]-V[i-1]) <0) tp++;
	}
	return((numtype)tp);
}
numtype TSShannonEntropy(int VLen, numtype* V) {
	int i;
	numtype* uval = (numtype*)malloc(VLen*sizeof(numtype));	// unique values
	numtype* uval_p = (numtype*)malloc(VLen*sizeof(numtype));	// unique values probabilities
	numtype h = 0;

	//-- Before anything else, make a local copy of Input Array
	numtype* tmpV = (numtype*)malloc(VLen*sizeof(numtype));
	memcpy(tmpV, V, VLen*sizeof(numtype));

	//-- First, sort input Array
	qsort(tmpV, VLen, sizeof(numtype), cmp_dbl);

	//-- Then, find unique values
	int unique = 1; //In case we have only one element; it is unique!
	for (i = 0; i < VLen; i++) {
		if (tmpV[i]==tmpV[i+1]) {
			uval_p[unique-1]++;
			continue;
		} else {
			unique++;
			uval[unique-1] = tmpV[i];
			uval_p[unique-1] = 1;
		}
	}

	//-- Normalize probabilities. Sum must be 1
	for (i = 0; i < (unique-1); i++) uval_p[i] = uval_p[i+1]/unique;

	//-- Then, calc entropy from p
	for (i = 0; i < unique; i++) h += uval_p[i]*log2(uval_p[i]);

	free(uval); free(uval_p); free(tmpV);
	return(h);
}
numtype TSHistoricalVolatility(int VLen, numtype* V) {

	//=========  BOGUS. NEED TO DOUBLE CHECK! ===========
	return (numtype)0.5;

	int i;
	numtype s, u;
	numtype* w = (numtype*)malloc(VLen*sizeof(numtype));

	s = 0;
	for (i = 1; i<VLen; i++) {
		u = log(V[i]/V[i-1]);
		if (i>1) {
			w[i] = w[i-1]*(numtype)0.94;
		} else {
			w[i] = (numtype)(1-0.94);
		}
		s += pow(u, 2)*w[i];
	}

	free(w);
	return sqrt(s);
}
