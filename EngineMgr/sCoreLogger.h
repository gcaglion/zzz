#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../Logger/sLogger.h"

struct sCoreLogger : sLogger {

	//-- What
	int logsCnt=4;
	bool saveMSEFlag;
	bool saveRunFlag;
	bool saveInternalsFlag;
	bool saveImageFlag;
	int readFrom=0;	//-- to remove!

	//-- manual
	sCoreLogger(sObjParmsDef, int readFrom_, bool saveToDB_, bool saveToFile_, bool saveMSEFlag_, bool saveRunFlag_, bool saveInternalsFlag_, bool saveImageFlag_);	
	//-- from XML
	sCoreLogger(sCfgObjParmsDef);
	//-- from DB
	EXPORT sCoreLogger(sObjParmsDef, sLogger* persistor_, int pid_, int tid_);
	~sCoreLogger();

	EXPORT void save(sLogger* persistor_, int pid_, int tid_);

private:
	char** ffn;
	void mallocs();
};