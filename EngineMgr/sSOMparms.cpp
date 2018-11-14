#include "sSOMparms.h"

sSOMparms::sSOMparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {}
sSOMparms::sSOMparms(sObjParmsDef, sLogger* persistor_, int loadingPid_, int loadingTid_) : sCoreParms(sObjParmsVal, persistor_, loadingPid_, loadingTid_) {
}
sSOMparms::~sSOMparms() {}

//-- local implementations of virtual functions defined in sCoreParms
void sSOMparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		scaleMin[l] = -1;
		scaleMax[l] = 1;
	}
}
void sSOMparms::save(sLogger* persistor_, int pid_, int tid_) {
	safecall(persistor_, saveCoreSOMparms, pid_, tid_, parm1, parm2);
}
