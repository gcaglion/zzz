#include "sGAparms.h"

sGAparms::sGAparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {
	levelsCnt=1;
}
sGAparms::sGAparms(sObjParmsDef, sLogger* persistor_, int loadingPid_, int loadingTid_) : sCoreParms(sObjParmsVal, persistor_, loadingPid_, loadingTid_) {
	levelsCnt=1;
}
sGAparms::~sGAparms() {}

//-- local implementations of virtual functions defined in sCoreParms
void sGAparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		scaleMin[l] = -1;
		scaleMax[l] = 1;
	}
}
void sGAparms::save(sLogger* persistor_, int pid_, int tid_) {
	safecall(persistor_, saveCoreGAparms, pid_, tid_, parm1, parm2);
}
