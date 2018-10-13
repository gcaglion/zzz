#pragma once
#include "../common.h"
#include "../ConfigMgr/sCfgObj.h"
#include "../DataMgr/DataShape.h"
#include "../DataMgr/DataSet.h"
#include "sCoreLayout.h"
#include "sCoreParms.h"
#include "Core_enums.h"

struct sCore : sCfgObj {
	int type;

	sCoreParms* parms;
	sCoreLayout* layout;
	sDataShape* baseDataShape;
	sDataSet* inputDS;
	sDataSet* outputDS;

	EXPORT sCore(sCfgObjParmsDef, sCoreLayout* layout_, sDataShape* dataShape_);
	EXPORT ~sCore();

	//-- methods common for all core subclasses


	//-- methods to be implemented indipendently by each subclass (sNN, sGA, ...)
	void sCore::loadInput(sDataSet* inputDS_) { inputDS=inputDS_; }
	virtual void train(sDataSet* ds_){}
	virtual void infer(sDataSet* ds_){}


};
