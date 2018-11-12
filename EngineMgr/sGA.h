#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sCoreLayout.h"
#include "sCore.h"
#include "sGAparms.h"

struct sGA : sCore {
	EXPORT sGA(sCfgObjParmsDef, sCoreLayout* layout_, sGAparms* GAparms_);
	EXPORT ~sGA();

	EXPORT void train(sCoreProcArgs* trainArgs);
	EXPORT void infer(sCoreProcArgs* inferArgs);

};