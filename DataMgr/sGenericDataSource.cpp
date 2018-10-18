#include "sGenericDataSource.h"

//=== sGenericDataSource
sGenericDataSource::sGenericDataSource(sCfgObjParmsDef, sFileData* file_, int fieldSep_, bool calcBW_, int BWfeatureH_, int BWfeatureL_, bool autoOpen) : sDataSource(sCfgObjParmsVal, FILE_SOURCE, NULL, calcBW_, BWfeatureH_, BWfeatureL_) {
	file=file_;
	db=nullptr;
	fieldSep=fieldSep_; calcBW=calcBW_; BWfeatureH=BWfeatureH_; BWfeatureL=BWfeatureL_;
	safecall(this, getFeaturesCnt, &featuresCnt);
	if (autoOpen) safecall(file, open, FILE_MODE_READ);
}
sGenericDataSource::sGenericDataSource(sCfgObjParmsDef, sOraData* db_, int fieldSep_, bool calcBW_, int BWfeatureH_, int BWfeatureL_, bool autoOpen) : sDataSource(sCfgObjParmsVal, DB_SOURCE, NULL, calcBW_, BWfeatureH_, BWfeatureL_) {
	file=nullptr;
	db=db_;
	fieldSep=fieldSep_; calcBW=calcBW_; BWfeatureH=BWfeatureH_; BWfeatureL=BWfeatureL_;
	safecall(this, getFeaturesCnt, &featuresCnt);
	if (autoOpen) safecall(db, open);
}
sGenericDataSource::sGenericDataSource(sCfgObjParmsDef, bool autoOpen) : sDataSource(sCfgObjParmsVal) {
	//-- TO DO !
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
