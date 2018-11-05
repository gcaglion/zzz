#include "sGenericDataSource.h"

//=== sGenericDataSource
sGenericDataSource::sGenericDataSource(sObjParmsDef, sFileData* file_, int fieldSep_, bool calcBW_, int BWfeatureH_, int BWfeatureL_, bool autoOpen) : sDataSource(sObjParmsVal, file_, NULL, calcBW_, BWfeatureH_, BWfeatureL_) {
	fieldSep=fieldSep_; calcBW=calcBW_; BWfeatureH=BWfeatureH_; BWfeatureL=BWfeatureL_;
	safecall(this, getFeaturesCnt, &featuresCnt);
}
sGenericDataSource::sGenericDataSource(sObjParmsDef, sOraData* db_, int fieldSep_, bool calcBW_, int BWfeatureH_, int BWfeatureL_, bool autoOpen) : sDataSource(sObjParmsVal, db_, NULL, calcBW_, BWfeatureH_, BWfeatureL_) {
	fieldSep=fieldSep_; calcBW=calcBW_; BWfeatureH=BWfeatureH_; BWfeatureL=BWfeatureL_;
	safecall(this, getFeaturesCnt, &featuresCnt);
}

sGenericDataSource::sGenericDataSource(sCfgObjParmsDef, bool autoOpen) : sDataSource(sCfgObjParmsVal) {
	if (autoOpen) open();
}

sGenericDataSource::~sGenericDataSource(){}

void sGenericDataSource::getFeaturesCnt(int* oFeaturesCnt_) {
	//.................
	(*oFeaturesCnt_)= 2;
	//.................
}

void sGenericDataSource::open() {
	fail("Not implemented!");
}
void sGenericDataSource::load(const char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar) {
	fail("Not implemented!");
}
