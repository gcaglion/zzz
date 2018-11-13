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

	//-- identical across core types
	EXPORT void saveMSE(int pid, int tid, int mseCnt, numtype* mseT, numtype* mseV);
	EXPORT void saveRun(int pid, int tid, int npid, int ntid, int runStepsCnt, int tsFeaturesCnt_, int selectedFeaturesCnt, int* selectedFeature, int predictionLen, char** posLabel, numtype* actualTRS, numtype* predictedTRS, numtype* actualTR, numtype* predictedTR, numtype* actual, numtype* predicted);
	EXPORT void commit();
	
	sCoreLogger(sObjParmsDef, bool saveToDB_, bool saveToFile_, bool saveMSEFlag_, bool saveRunFlag_, bool saveInternalsFlag_, bool saveImageFlag_);
	sCoreLogger(sCfgObjParmsDef);
	~sCoreLogger();

private:
	char** ffn;
	void mallocs();
};