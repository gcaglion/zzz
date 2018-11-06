#include "sDataSource.h"


//=== sDataSource
sDataSource::sDataSource(sObjParmsDef, sOraData* oradb_, int featuresCnt_, bool calcBW_, int BWfeatureH_, int BWfeatureL_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	filedb=nullptr; oradb=oradb_; featuresCnt=featuresCnt_; calcBW=calcBW_; BWfeatureH=BWfeatureH_; BWfeatureL=BWfeatureL_;
}
sDataSource::sDataSource(sObjParmsDef, sFileData* filedb_, int featuresCnt_, bool calcBW_, int BWfeatureH_, int BWfeatureL_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {
	filedb=filedb_; oradb=nullptr; featuresCnt=featuresCnt_; calcBW=calcBW_; BWfeatureH=BWfeatureH_; BWfeatureL=BWfeatureL_;
}
sDataSource::sDataSource(sObjParmsDef, sMT4Data* MT4db_, int featuresCnt_, bool calcBW_, int BWfeatureH_, int BWfeatureL_) : sCfgObj(sObjParmsVal, nullptr, nullptr) {

}
sDataSource::sDataSource(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {}

sDataSource::~sDataSource(){}