#pragma once
#include "../common.h"
#include "sDataSource.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../ConfigMgr/sCfg.h"
#include "FXData_enums.h"

typedef struct sMT4Data : public sDataSource {
	int accountId;	//-- sarca cosa mi serve qui...

	EXPORT sMT4Data(sCfgObjParmsDef, int accountId_, bool autoOpen);
	EXPORT sMT4Data(sCfgObjParmsDef, bool autoOpen);
	EXPORT ~sMT4Data();

} tMT4Data;
