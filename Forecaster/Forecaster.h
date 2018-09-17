#pragma once

#include "../common.h"
#include "../ConfigMgr/sCfg.h"
#include "../DataMgr/sData.h"
//#include "../Engine/Engine.h"
//#include "../Logger/Logger.h"

struct sForecaster : sCfgObj {
	
	sData* data=nullptr;
	//tEngine* engine=nullptr;
	//tLogger* persistor=nullptr;

	EXPORT sForecaster(sCfgObjParmsDef, sData* forecastData_);
	EXPORT sForecaster(sCfgObjParmsDef);
	EXPORT ~sForecaster();

};