#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"

#include "../DataMgr/FileData.h"

#define FILEDB_MAX_FILES	16

struct sFileDB : sCfgObj {

	sFileData* datafile[FILEDB_MAX_FILES];

	EXPORT sFileDB(sCfgObjParmsDef);
	EXPORT ~sFileDB();

	EXPORT void getStartDates(sFileData* dateSource_, char* startDate_, int datesCnt_, char** oDate_);

};