#include "sMT4DataSource.h"

sMT4DataSource::sMT4DataSource(sObjParmsDef, int sampleLen_, long* iBarT, double* iBarO, double* iBarH, double* iBarL, double* iBarC, double* iBarV, long iBaseBarT, double iBaseBarO, double iBaseBarH, double iBaseBarL, double iBaseBarC, double iBaseBarV) : sDataSource(sObjParmsVal, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
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
	basebar[FXOPEN]=(numtype)iBaseBarO;
	basebar[FXHIGH]=(numtype)iBaseBarH;
	basebar[FXLOW]=(numtype)iBaseBarL;
	basebar[FXCLOSE]=(numtype)iBaseBarC;
	basebar[FXVOLUME]=(numtype)iBaseBarV;


	//-- 2. convert bar time
	time_t kaz;
	struct tm buf;

	kaz=(time_t)iBaseBarT;
	localtime_s(&buf, &kaz);
	strftime(basetime, DATE_FORMAT_LEN, DATE_FORMAT_C, &buf);
	info("converted iBaseBarT=%s", basetime);

	for (int b=0; b<sampleLen_; b++) {
		kaz=(time_t)iBarT[b];
		localtime_s(&buf, &kaz);
		strftime(bartime[b], DATE_FORMAT_LEN, DATE_FORMAT_C, &buf);
		info("converted iBarT[%d]=%s", b, bartime[b]);
	}
	strcpy_s(lastbartime, DATE_FORMAT_LEN, bartime[sampleLen-1]);

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
	
	for(int b=0; b<sampleLen; b++) strcpy_s(oBarTime[b], DATE_FORMAT_LEN, bartime[b]);
	memcpy_s(oBarData, sampleLen*FXDATA_FEATURESCNT*sizeof(numtype), sample, sampleLen*FXDATA_FEATURESCNT*sizeof(numtype));
	strcpy_s(oBaseTime, DATE_FORMAT_LEN, basetime);
	memcpy_s(oBaseBar, FXDATA_FEATURESCNT*sizeof(numtype), basebar, FXDATA_FEATURESCNT*sizeof(numtype));
}
