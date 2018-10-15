#include "sFileData.h"

//=== sFileData
sFileData::sFileData(sCfgObjParmsDef, FILE* srcFile_, int fieldSep_, bool calcBW_, int BWfeatureH_, int BWfeatureL_, bool autoOpen) : sDataSource(sCfgObjParmsVal, FILE_SOURCE, NULL, calcBW_, BWfeatureH_, BWfeatureL_) {
	srcFile=srcFile_; fieldSep=fieldSep_;
	safecall(this, getFeaturesCnt, &featuresCnt);
}
sFileData::sFileData(sCfgObjParmsDef, bool autoOpen) : sDataSource(sCfgObjParmsVal, FILE_SOURCE, NULL, NULL, NULL, NULL) {

}
sFileData::~sFileData(){}

void sFileData::getFeaturesCnt(int* oFeaturesCnt_) {
	//.................
	(*oFeaturesCnt_)= 2;
	//.................
}

void sFileData::open() {
	fail("Not implemented!");
}
void sFileData::load(const char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar) {
	fail("Not implemented!");
}
