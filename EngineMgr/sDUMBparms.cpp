#include "sDUMBparms.h"

sDUMBparms::sDUMBparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {
	levelsCnt=1;
}
sDUMBparms::sDUMBparms(sObjParmsDef, sLogger* persistor_, int loadingPid_) : sCoreParms(sObjParmsVal, persistor_, loadingPid_) {
	levelsCnt=1;
}
sDUMBparms::~sDUMBparms() {}

//-- local implementations of virtual functions defined in sCoreParms
void sDUMBparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		scaleMin[l] = -1;
		scaleMax[l] = 1;
	}
}
void sDUMBparms::save(int pid, int tid) {}
void sDUMBparms::load(int pid, int tid) {}
