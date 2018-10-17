#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"

#define FILEDATA_MAX_FILES	16

struct sFileData : sCfgObj {

	int filesCnt;
	char** fileFullName;	// one file for each log (Client, MSE, Run, Internals, Image)

	EXPORT sFileData(sCfgObjParmsDef);
	EXPORT ~sFileData();

	EXPORT void getStartDates(sFileData* dateSource_, char* startDate_, int datesCnt_, char** oDate_);

private:
	void ffname_malloc();

};