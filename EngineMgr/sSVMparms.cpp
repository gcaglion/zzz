#include "sSVMparms.h"

sSVMparms::sSVMparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {}
sSVMparms::sSVMparms(sObjParmsDef, sLogger* persistor_, int loadingPid_) : sCoreParms(sObjParmsVal, persistor_, loadingPid_) {
}
sSVMparms::~sSVMparms(){}

//-- local implementations of virtual functions defined in sCoreParms
void sSVMparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		scaleMin[l] = -1;
		scaleMax[l] = 1;
	}
}
void sSVMparms::save(int pid, int tid) {}
void sSVMparms::load(int pid, int tid) {}
