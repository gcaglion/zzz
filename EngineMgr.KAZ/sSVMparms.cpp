#include "sSVMparms.h"

sSVMparms::sSVMparms(sCfgObjParmsDef) : sCoreParms(sCfgObjParmsVal) {}
sSVMparms::sSVMparms(sObjParmsDef) : sCoreParms(sObjParmsVal, nullptr, nullptr) {}
sSVMparms::~sSVMparms(){}


void sSVMparms::setScaleMinMax() {
	for (int l=0; l<levelsCnt; l++) {
		scaleMin[l] = -1;
		scaleMax[l] = 1;
	}
}
