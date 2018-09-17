#pragma once
#include "../common.h"
#include "DataSource.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../ConfigMgr/sCfg.h"
#include "FXData_enums.h"

typedef struct sMT4Data : public sDataSource {
	int accountId;	//-- sarca cosa mi serve qui...

	EXPORT sMT4Data(sCfgObjParmsDef, int accountId_);
	EXPORT sMT4Data(sCfgObjParmsDef);
	EXPORT ~sMT4Data();

} tMT4Data;
