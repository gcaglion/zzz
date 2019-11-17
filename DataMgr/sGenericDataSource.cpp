#include "sGenericDataSource.h"

//=== sGenericDataSource
sGenericDataSource::sGenericDataSource(sObjParmsDef, sFileData* file_, int fieldSep_, bool calcBW_, int BWfeatureH_, int BWfeatureL_) : sDataSource(sObjParmsVal, -99, calcBW_, BWfeatureH_, BWfeatureL_) {
	type=FILE_SOURCE; filedb=file_;

	fieldSep=fieldSep_; calcBW=calcBW_; BWfeatureH=BWfeatureH_; BWfeatureL=BWfeatureL_;
	safecall(this, getFeaturesCnt, &featuresCnt);
}
sGenericDataSource::sGenericDataSource(sObjParmsDef, sOraData* db_, int fieldSep_, bool calcBW_, int BWfeatureH_, int BWfeatureL_) : sDataSource(sObjParmsVal, -99, calcBW_, BWfeatureH_, BWfeatureL_) {
	type=DB_SOURCE; oradb=db_;
	
	fieldSep=fieldSep_; calcBW=calcBW_; BWfeatureH=BWfeatureH_; BWfeatureL=BWfeatureL_;
	safecall(this, getFeaturesCnt, &featuresCnt);
}
sGenericDataSource::sGenericDataSource(sCfgObjParmsDef) : sDataSource(sCfgObjParmsVal){}

sGenericDataSource::~sGenericDataSource(){}

void sGenericDataSource::getFeaturesCnt(int* oFeaturesCnt_) {
	//.................
	(*oFeaturesCnt_)= 2;
	//.................
}

void sGenericDataSource::open() {
	fail("Not implemented!");
}
void sGenericDataSource::load(char* pDate0, int pDate0Lag, int pRecCount, char** oBarTime, numtype* oBarData, char* oBaseTime, numtype* oBaseBar, numtype* oBarWidth) {
	fail("Not implemented!");
}
