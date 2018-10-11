#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/DataShape.h"
#include "../DataMgr/DataSet.h"
#include "sCoreLayout.h"
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
	sCore** core;

	EXPORT sEngine(sCfgObjParmsDef, sDataShape* dataShape_);
	EXPORT ~sEngine();

	EXPORT void setCoreLayer(sCore* c);
	EXPORT void train(sDataSet* trainDS_);
	EXPORT void infer(sDataSet* testDS_);

};