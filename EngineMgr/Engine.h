#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/DataShape.h"
#include "../DataMgr/DataSet.h"
#include "sCore.h"
#include "Engine_enums.h"
#include "cuNN.h"

#define MAX_ENGINE_LAYERS	8
#define MAX_ENGINE_CORES	32

struct sEngine : sCfgObj {

	int type;
	int coresCnt;
	int layersCnt=0;
	int* layerCoresCnt;

	sDataShape* dataShape;
	sCoreLayout** coreLayout;
	sCore** core;

	EXPORT sEngine(sCfgObjParmsDef, sDataShape* dataShape_);
	EXPORT ~sEngine();

	EXPORT void setCoreLayer(sCoreLayout* c);
	EXPORT void train(sDataSet* trainDS);
	EXPORT void addCore(tParmsSource* parms, int coreId);
	EXPORT void cleanup();

};