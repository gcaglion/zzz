#include "sSOMparms.h"

sSOMparms::sSOMparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {}
sSOMparms::sSOMparms(sObjParmsDef, sLogger* persistor_, int loadingPid_) : sCoreParms(sObjParmsVal, persistor_, loadingPid_) {
}
sSOMparms::~sSOMparms() {}

//-- local implementations of virtual functions defined in sCoreParms
void sSOMparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		scaleMin[l] = -1;
		scaleMax[l] = 1;
	}
}
void sSOMparms::save(int pid, int tid) {}
void sSOMparms::load(int pid, int tid) {}
