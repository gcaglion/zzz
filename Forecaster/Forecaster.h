#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/sData.h"
#include "../EngineMgr/sEngine.h"
#include "../Logger/sLogger.h"

struct sForecaster : sCfgObj {
	
	sData* data;
	sEngine* engine;
	sLogger* persistor;

	EXPORT sForecaster(sCfgObjParmsDef, sData* data_, sEngine* engine_, sLogger* persistor_);
	EXPORT sForecaster(sCfgObjParmsDef);
	EXPORT ~sForecaster();

	void getForecast(char* trainDate0_=nullptr, char* testDate0_=nullptr, char* validDate0_=nullptr);	//-- if any of these is nullptr, then we use date0 from cfg dedicated section
};

