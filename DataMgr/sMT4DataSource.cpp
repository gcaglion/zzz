#include "sMT4DataSource.h"

sMT4DataSource::sMT4DataSource(sCfgObjParmsDef, int accountId_, bool autoOpen) : sDataSource(sCfgObjParmsVal, MT4_SOURCE, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	accountId=accountId_;
}
sMT4DataSource::sMT4DataSource(sCfgObjParmsDef, bool autoOpen) : sDataSource(sCfgObjParmsVal, MT4_SOURCE, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	// accountId= getparm(...)
}

sMT4DataSource::~sMT4DataSource(){}
