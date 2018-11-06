#include "sDataSource.h"


//=== sDataSource
sDataSource::sDataSource(sObjParmsDef, int featuresCnt_, bool calcBW_, int BWfeatureH_, int BWfeatureL_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	featuresCnt=featuresCnt_; calcBW=calcBW_; BWfeatureH=BWfeatureH_; BWfeatureL=BWfeatureL_;
	isOpen=false;
}
sDataSource::sDataSource(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
	isOpen=false;
}

sDataSource::~sDataSource() {}

void sDataSource::getStartDates(char* date0_, int datesCnt_, char** oStartDates_) {}
void sDataSource::open() {
	isOpen=true;
}