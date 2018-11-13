#pragma once

#include "../common.h"
#include "../BaseObj/sObj.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../FileData/sFileData.h" 
#include "../OraData/sOraData.h"

struct sLogger : sCfgObj {
	
	//-- How many logs
	int logsCnt;

	//-- Where
	bool saveToDB;
	bool saveToFile;
	sOraData* oradb;
	sFileData* filedb;

	EXPORT sLogger(sObjParmsDef);
	EXPORT sLogger(sCfgObjParmsDef);
	EXPORT ~sLogger();

	EXPORT virtual void open();
	EXPORT virtual void commit() {}

};