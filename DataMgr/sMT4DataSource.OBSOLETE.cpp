#include "sMT4DataSource.h"

sMT4DataSource::sMT4DataSource(sObjParmsDef, int historyLen_, int predictionLen_, long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, long iBaseBarT, double iBaseBarO, double iBaseBarH, double iBaseBarL, double iBaseBarC, double iBaseBarV) : sDataSource(sObjParmsVal, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	historyLen=historyLen_; predictionLen=predictionLen_;

	//-- 0. mallocs
	bartime=(char**)malloc((historyLen+predictionLen)*sizeof(char*)); for (int b=0; b<(historyLen+predictionLen); b++) bartime[b]=(char*)malloc(DATE_FORMAT_LEN);

	//-- 1. transform input into a flat 2D array (BF-ordered) with all 5 features
	sample=(numtype*)malloc((historyLen+predictionLen)*FXDATA_FEATURESCNT*sizeof(numtype));

	for (int b=0; b<historyLen; b++) {
		sample[b*FXDATA_FEATURESCNT+0]=(numtype)iBarO[b];
		sample[b*FXDATA_FEATURESCNT+1]=(numtype)iBarH[b];
		sample[b*FXDATA_FEATURESCNT+2]=(numtype)iBarL[b];
		sample[b*FXDATA_FEATURESCNT+3]=(numtype)iBarC[b];
		sample[b*FXDATA_FEATURESCNT+4]=(numtype)iBarV[b];
	}
	//-- add base bar
	basebar[FXOPEN]=(numtype)iBaseBarO;
	basebar[FXHIGH]=(numtype)iBaseBarH;
	basebar[FXLOW]=(numtype)iBaseBarL;
	basebar[FXCLOSE]=(numtype)iBaseBarC;
	basebar[FXVOLUME]=(numtype)iBaseBarV;


	//-- 2. convert bar time
	MT4time2str(iBaseBarT, DATE_FORMAT_LEN, basetime);

	for (int b=0; b<historyLen; b++) MT4time2str(iBarT[b], DATE_FORMAT_LEN, bartime[b]);
	strcpy_s(lastbartime, DATE_FORMAT_LEN, bartime[historyLen-1]);

	info("baseBar: %s %f,%f,%f,%f,%f", basetime, basebar[FXOPEN], basebar[FXHIGH], basebar[FXLOW], basebar[FXCLOSE], basebar[FXVOLUME]);
	for (int b=0; b<historyLen; b++) info("Bar[%d]: %s %f,%f,%f,%f,%f", b, bartime[b], sample[b*FXDATA_FEATURESCNT+FXOPEN], sample[b*FXDATA_FEATURESCNT+FXHIGH], sample[b*FXDATA_FEATURESCNT+FXLOW], sample[b*FXDATA_FEATURESCNT+FXCLOSE], sample[b*FXDATA_FEATURESCNT+FXVOLUME]);
}
sMT4DataSource::sMT4DataSource(sObjParmsDef, sMT4Data* MT4db_) : sDataSource(sObjParmsVal, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	mt4db=MT4db_;
}
sMT4DataSource::sMT4DataSource(sCfgObjParmsDef) : sDataSource(sCfgObjParmsVal) {
	// accountId= getparm(...)
}

sMT4DataSource::~sMT4DataSource(){
	free(sample);
	for (int b=0; b<(historyLen+predictionLen); b++) free(bartime[b]);
	free(bartime);
}

void sMT4DataSource::load(const char* pDate0, int pRecCount, char** oBarTime, numtype* oBarData, char* oBaseTime, numtype* oBaseBar, numtype* oBarWidth) {
	
	for(int b=0; b<historyLen; b++) strcpy_s(oBarTime[b], DATE_FORMAT_LEN, bartime[b]);
	memcpy_s(oBarData, historyLen*FXDATA_FEATURESCNT*sizeof(numtype), sample, historyLen*FXDATA_FEATURESCNT*sizeof(numtype));
	
	strcpy_s(oBaseTime, DATE_FORMAT_LEN, basetime);
	memcpy_s(oBaseBar, FXDATA_FEATURESCNT*sizeof(numtype), basebar, FXDATA_FEATURESCNT*sizeof(numtype));

	//-- empty steps
	for (int bar=historyLen; bar<(historyLen+predictionLen); bar++) {
		for (int f=0; f<FXDATA_FEATURESCNT; f++) {
			oBarData[bar*FXDATA_FEATURESCNT+f]=EMPTY_VALUE;
		}
	}
}
