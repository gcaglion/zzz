#include "sDataSource.h"


//=== sDataSource
sDataSource::sDataSource(sObjParmsDef, int featuresCnt_, bool calcBW_, int BWfeatureH_, int BWfeatureL_) : sCfgObj(sObjParmsVal, nullptr, "") {
	featuresCnt=featuresCnt_; calcBW=calcBW_; BWfeatureH=BWfeatureH_; BWfeatureL=BWfeatureL_;
}
sDataSource::sDataSource(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
}

sDataSource::~sDataSource() {}

void sDataSource::getStartDates(char* date0_, int datesCnt_, char*** oStartDates_) {}
void sDataSource::open() {
}