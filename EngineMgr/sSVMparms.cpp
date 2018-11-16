#include "sSVMparms.h"

sSVMparms::sSVMparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {}
sSVMparms::sSVMparms(sObjParmsDef, sLogger* persistor_, int loadingPid_, int loadingTid_) : sCoreParms(sObjParmsVal, persistor_, loadingPid_, loadingTid_) {
}
sSVMparms::~sSVMparms(){}

//-- local implementations of virtual functions defined in sCoreParms
void sSVMparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		scaleMin[l] = -1;
		scaleMax[l] = 1;
	}
}
void sSVMparms::save(sLogger* persistor_, int pid_, int tid_) {
	safecall(persistor_, saveCoreSVMparms, pid_, tid_, parm1, parm2);
}
