#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../OraData/sOraData.h"
#include "../FileData/sFileData.h"

struct sCoreLogger : sCfgObj {

	sOraData* db;
	sFileData* file;

	//-- Where
	bool saveToDB;
	bool saveToFile;
	//-- What
	bool saveMSEFlag;
	bool saveRunFlag;
	bool saveInternalsFlag;
	bool saveImageFlag;

	EXPORT void saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV);
	EXPORT void saveRun();

	sCoreLogger(sObjParmsDef, bool saveToDB_, bool saveToFile_, bool saveMSEFlag_, bool saveRunFlag_, bool saveInternalsFlag_, bool saveImageFlag_);
	sCoreLogger(sCfgObjParmsDef);
	~sCoreLogger();

};