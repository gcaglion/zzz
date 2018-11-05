#pragma once
#include "../common.h"
#include "sDataSource.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../ConfigMgr/sCfg.h"
#include "FXData_enums.h"
#include "../MT4Data/sMT4Data.h"

typedef struct sMT4DataSource : sDataSource {

	sMT4Data* mt4db;

	EXPORT sMT4DataSource(sObjParmsDef, sMT4Data* mt4db_, bool autoOpen);
	EXPORT sMT4DataSource(sCfgObjParmsDef, bool autoOpen);
	EXPORT ~sMT4DataSource();

} tMT4Data;
