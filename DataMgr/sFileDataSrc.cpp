#include "sFileDataSrc.h"

//=== sFileDataSrc
sFileDataSrc::sFileDataSrc(sCfgObjParmsDef, FILE* srcFile_, int fieldSep_, bool calcBW_, int BWfeatureH_, int BWfeatureL_, bool autoOpen) : sDataSource(sCfgObjParmsVal, FILE_SOURCE, NULL, calcBW_, BWfeatureH_, BWfeatureL_) {
	srcFile=srcFile_; fieldSep=fieldSep_;
	safecall(this, getFeaturesCnt, &featuresCnt);
}
sFileDataSrc::sFileDataSrc(sCfgObjParmsDef, bool autoOpen) : sDataSource(sCfgObjParmsVal, FILE_SOURCE, NULL, NULL, NULL, NULL) {

}
sFileDataSrc::~sFileDataSrc(){}

void sFileDataSrc::getFeaturesCnt(int* oFeaturesCnt_) {
	//.................
	(*oFeaturesCnt_)= 2;
	//.................
}

void sFileDataSrc::open() {
	fail("Not implemented!");
}
void sFileDataSrc::load(const char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar) {
	fail("Not implemented!");
}
