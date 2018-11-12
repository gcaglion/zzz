#pragma once
#include "../common.h"
#include "sDataSource.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../ConfigMgr/sCfg.h"
#include "FXData_enums.h"
#include "../MT4Data/sMT4Data.h"

struct sMT4DataSource : sDataSource {

	EXPORT sMT4DataSource(sObjParmsDef, sMT4Data* mt4db_);
	EXPORT sMT4DataSource(sCfgObjParmsDef);
	EXPORT ~sMT4DataSource();

};
