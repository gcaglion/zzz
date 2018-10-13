#include "sMT4Data.h"

sMT4Data::sMT4Data(sCfgObjParmsDef, int accountId_) : sDataSource(sCfgObjParmsVal, MT4_SOURCE, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	accountId=accountId_;
}
sMT4Data::sMT4Data(sCfgObjParmsDef) : sDataSource(sCfgObjParmsVal, MT4_SOURCE, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	// accountId= getparm(...)
}

sMT4Data::~sMT4Data(){}
