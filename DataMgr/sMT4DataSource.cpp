#include "sMT4DataSource.h"

sMT4DataSource::sMT4DataSource(sObjParmsDef, int sampleLen_, int* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, double iBaseBarO, double iBaseBarH, double iBaseBarL, double iBaseBarC, double iBaseBarV) : sDataSource(sObjParmsVal, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	sampleLen=sampleLen_;

	//-- 0. mallocs
	bartime=(char**)malloc(sampleLen_*sizeof(char*)); for (int b=0; b<sampleLen_; b++) bartime[b]=(char*)malloc(DATE_FORMAT_LEN);

	//-- 1. transform input into a flat 2D array (BF-ordered) with all 5 features
	sample=(numtype*)malloc(sampleLen_*FXDATA_FEATURESCNT*sizeof(numtype));

	for (int b=0; b<sampleLen_; b++) {
		sample[b*FXDATA_FEATURESCNT+0]=(numtype)iBarO[b];
		sample[b*FXDATA_FEATURESCNT+1]=(numtype)iBarH[b];
		sample[b*FXDATA_FEATURESCNT+2]=(numtype)iBarL[b];
		sample[b*FXDATA_FEATURESCNT+3]=(numtype)iBarC[b];
		sample[b*FXDATA_FEATURESCNT+4]=(numtype)iBarV[b];
	}
	//-- add base bar
	basebar[FXOPEN]=iBaseBarO;
	basebar[FXHIGH]=iBaseBarH;
	basebar[FXLOW]=iBaseBarL;
	basebar[FXCLOSE]=iBaseBarC;
	basebar[FXVOLUME]=iBaseBarV;


	//-- 2. convert bar time (TODO!)
	for (int b=0; b<sampleLen_; b++) {
		sprintf_s(bartime[b], DATE_FORMAT_LEN, "%d", iBarT[b]);
	}
}
sMT4DataSource::sMT4DataSource(sObjParmsDef, sMT4Data* MT4db_) : sDataSource(sObjParmsVal, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	mt4db=MT4db_;
}
sMT4DataSource::sMT4DataSource(sCfgObjParmsDef) : sDataSource(sCfgObjParmsVal) {
	// accountId= getparm(...)
}

sMT4DataSource::~sMT4DataSource(){
	free(sample);
	for (int b=0; b<sampleLen; b++) free(bartime[b]);
	free(bartime);
}

void sMT4DataSource::load(char* pDate0, int pRecCount, char** oBarTime, numtype* oBarData, char* oBaseTime, numtype* oBaseBar, numtype* oBarWidth) {
	
	oBarTime=bartime;
	oBarData=sample;
	//oBaseTime=
}
