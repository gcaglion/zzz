#include "sMT4DataSource.h"

sMT4DataSource::sMT4DataSource(sObjParmsDef, sMT4Data* mt4db_, bool autoOpen) : sDataSource(sObjParmsVal, mt4db_, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	mt4db=mt4db_;
}
sMT4DataSource::sMT4DataSource(sCfgObjParmsDef, bool autoOpen) : sDataSource(sCfgObjParmsVal) {
	// accountId= getparm(...)
}

sMT4DataSource::~sMT4DataSource(){}
