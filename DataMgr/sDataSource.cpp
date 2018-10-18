#include "sDataSource.h"


//=== sDataSource
sDataSource::sDataSource(sCfgObjParmsDef, int type_, int featuresCnt_, bool calcBW_, int BWfeatureH_, int BWfeatureL_) : sCfgObj(sCfgObjParmsVal) {
	type=type_; featuresCnt=featuresCnt_; calcBW=calcBW_; BWfeatureH=BWfeatureH_; BWfeatureL=BWfeatureL_;
}
sDataSource::sDataSource(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {}

sDataSource::~sDataSource() {}