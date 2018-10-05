#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfg.h"
#include "../DataMgr/sData.h"
#include "../EngineMgr/Engine.h"
#include "../Logger/Logger.h"

struct sForecaster : sCfgObj {
	
	sData* data;
	sEngine* engine;
	sLogger* persistor;

	EXPORT sForecaster(sCfgObjParmsDef, sData* data_, sEngine* engine_, sLogger* persistor_);
	EXPORT sForecaster(sCfgObjParmsDef);
	EXPORT ~sForecaster();

};