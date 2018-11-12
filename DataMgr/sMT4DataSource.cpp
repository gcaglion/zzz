#include "sMT4DataSource.h"

sMT4DataSource::sMT4DataSource(sObjParmsDef, sMT4Data* MT4db_) : sDataSource(sObjParmsVal, FXDATA_FEATURESCNT, true, FXHIGH, FXLOW) {
	mt4db=MT4db_;
}
sMT4DataSource::sMT4DataSource(sCfgObjParmsDef) : sDataSource(sCfgObjParmsVal) {
	// accountId= getparm(...)
}

sMT4DataSource::~sMT4DataSource(){}
