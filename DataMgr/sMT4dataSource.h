#pragma once
#include "../common.h"
#include "sDataSource.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../ConfigMgr/sCfg.h"
#include "FXData_enums.h"

typedef struct sMT4DataSource : public sDataSource {
	int accountId;	//-- sarca cosa mi serve qui...

	EXPORT sMT4DataSource(sCfgObjParmsDef, int accountId_, bool autoOpen);
	EXPORT sMT4DataSource(sCfgObjParmsDef, bool autoOpen);
	EXPORT ~sMT4DataSource();

} tMT4Data;
