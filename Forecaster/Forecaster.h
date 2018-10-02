#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfg.h"
#include "../DataMgr/sData.h"
//#include "../Engine/Engine.h"
#include "../Logger/Logger.h"

struct sForecaster : sCfgObj {
	
	sData* data;
	//tEngine* engine;
	sLogger* persistor;

	EXPORT sForecaster(sCfgObjParmsDef, sData* data_, /*sEngine* engine_, */sLogger* persistor_);
	EXPORT sForecaster(sCfgObjParmsDef);
	EXPORT ~sForecaster();

};