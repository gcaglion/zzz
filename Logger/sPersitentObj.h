#pragma once
#include "../ConfigMgr/sCfgObj.h"
#include "sLogger.h"

struct sPersistentObj : sCfgObj {

	sLogger* persistor;

	sPersistentObj(sCfgObjParmsDef) : sCfgObj(sCfgObjParmsVal) {
		//-- load persistor info from cfg HERE.
		//-- load object info from obj cfg in object constructor
	}

	sPersistentObj(sCfgObjParmsDef, int loadFromPid_) : sCfgObj(sCfgObjParmsVal) {
		//-- load persistor info from cfg HERE.
		//-- load object info from persistor.
		//-- object constructor does nothing else
	}

	sPersistentObj(sCfgObjParmsDef, sLogger* persistor_) : sCfgObj(sCfgObjParmsVal) {
		//-- set persistor info from parameter HERE.
		//-- load object info from obj cfg in object constructor
		//-- 
	}

	sPersistentObj(sCfgObjParmsDef, sLogger* persistor_, int loadFromPid_) : sCfgObj(sCfgObjParmsVal) {
		//-- set persistor info from parameter HERE.
		//-- load object info from persistor.
		//-- object constructor does nothing else
	}

};