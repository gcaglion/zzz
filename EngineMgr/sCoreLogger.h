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

	EXPORT void saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV);
	EXPORT void saveRun();
	EXPORT void commit();
	//--
	EXPORT void loadW(int pid, int tid, int epoch, int Wcnt, numtype* W);

	sCoreLogger(sObjParmsDef, bool saveToDB_, bool saveToFile_, bool saveMSEFlag_, bool saveRunFlag_, bool saveInternalsFlag_, bool saveImageFlag_);
	sCoreLogger(sCfgObjParmsDef);
	~sCoreLogger();

private:
	char** ffn;
	void mallocs();
};